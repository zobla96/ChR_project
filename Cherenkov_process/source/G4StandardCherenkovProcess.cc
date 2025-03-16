//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//G4 headers
#include "G4StandardCherenkovProcess.hh"
#include "G4StandardChRProcess_Messenger.hh"
#include "G4SystemOfUnits.hh"
#include "GlobalFunctions.hh"
#include "G4AccessPhysicsVectors.hh"
#include "G4OpticalParameters.hh"
#include "G4LossTableManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4Poisson.hh"
#include "G4LogicalVolumeStore.hh"

#define pow2(x) ((x) * (x))

//two static help methods based around this translation unit - definition at the end of the file
static void PrintSimpleTables(const G4Material*, const std::vector<G4ChRPhysTableData::G4AroundBetaBasedValues>&);
static void PrintMoreComplexTables(const unsigned char, const G4Material*, const G4ChRPhysTableData&);

G4StandardCherenkovProcess::G4ChRPhysicsTableVector G4StandardCherenkovProcess::m_ChRPhysDataVec{};
unsigned int G4StandardCherenkovProcess::m_noOfBetaSteps = 20;

//=========public G4CherenkovProcess:: methods=========

G4StandardCherenkovProcess::G4StandardCherenkovProcess(const G4String& name)
: G4VDiscreteProcess(name, fElectromagnetic),  m_useEnergyLoss(false) {
	p_particleChange = new G4ParticleChange{};
	p_ChRProcessMessenger = new G4StandardChRProcess_Messenger{ this };
}

G4StandardCherenkovProcess::~G4StandardCherenkovProcess() {
	delete p_particleChange;
	delete p_ChRProcessMessenger;
}

G4double G4StandardCherenkovProcess::PostStepGetPhysicalInteractionLength(const G4Track& aTrack, G4double, G4ForceCondition* condition) {
	*condition = NotForced;
	G4double stepLimit = DBL_MAX;
	size_t matIndex = aTrack.GetMaterial()->GetIndex();

	if (m_ChRPhysDataVec.size() != G4Material::GetNumberOfMaterials()) {
		const char* err = "Not all materials have been registered in Cherenkov physics tables!\n";
		G4Exception("G4StandardCherenkovProcess::PostStepModelIntLength", "FE_stdChRProc01", FatalException, err);
	}
	G4ChRPhysTableData& physData = m_ChRPhysDataVec[matIndex];
	if (physData.m_aroundBetaValues.size() <= 1)
		return stepLimit;
	
	const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
	const G4MaterialCutsCouple* couple = aTrack.GetMaterialCutsCouple();

	G4double kineticEnergy = aParticle->GetKineticEnergy();
	const G4ParticleDefinition* particleType = aParticle->GetDefinition();
	G4double mass = particleType->GetPDGMass();

	G4double beta = aParticle->GetTotalMomentum() / aParticle->GetTotalEnergy();
	G4double gamma = aParticle->GetTotalEnergy() / mass;
	G4double gammaMin = 1 / std::sqrt(1. - pow2(physData.m_aroundBetaValues.front().m_betaValue));

	if (beta <= physData.m_aroundBetaValues.front().m_betaValue)
		return stepLimit;

	G4double kinEmin = mass * (gammaMin - 1.);
	G4double RangeMin = G4LossTableManager::Instance()->GetRange(particleType, kinEmin, couple);
	G4double Range = G4LossTableManager::Instance()->GetRange(particleType, kineticEnergy, couple);
	G4double step = Range - RangeMin;

	// If the step is smaller than G4ThreeVector::getTolerance(), it may happen
	// that the particle does not move. See bug 1992.
	if (step < G4ThreeVector::getTolerance())
		return stepLimit;

	if (step < stepLimit)
		stepLimit = step;

	G4OpticalParameters* optParameters = G4OpticalParameters::Instance();
	// If user has defined an average maximum number of photons to be generated in
	// a Step, then calculate the Step length for that number of photons.
	if (optParameters->GetCerenkovMaxPhotonsPerStep() > 0) {
		const G4double charge = aParticle->GetDefinition()->GetPDGCharge();
		G4double meanNumberOfPhotons = CalculateAverageNumberOfPhotons(charge, beta, matIndex);
		step = 0.;
		if (meanNumberOfPhotons > 0.0)
			step = optParameters->GetCerenkovMaxPhotonsPerStep() / meanNumberOfPhotons;
		if (step > 0. && step < stepLimit)
			stepLimit = step;
	}

	// If user has defined an maximum allowed change in beta per step
	if (optParameters->GetCerenkovMaxBetaChange() > 0.) {
		G4double dedx = G4LossTableManager::Instance()->GetDEDX(particleType, kineticEnergy, couple);
		G4double deltaGamma = gamma - 1. / std::sqrt(1. - pow2((1. - optParameters->GetCerenkovMaxBetaChange()) * beta));

		step = mass * deltaGamma / dedx;
		if (step > 0. && step < stepLimit)
			stepLimit = step;
	}

	*condition = StronglyForced;
	return stepLimit;
}
#include "G4AnalysisManager.hh"
G4VParticleChange* G4StandardCherenkovProcess::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) {
	p_particleChange->Initialize(aTrack);
	const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
	const G4Material* aMaterial = aTrack.GetMaterial();
	size_t materialID = aMaterial->GetIndex();

	//NOTE: no need for various checking, bcs the PostStepIntLNG would kill it
	//already if something was off

	const G4StepPoint* preStepPoint = aStep.GetPreStepPoint();
	const G4StepPoint* postStepPoint = aStep.GetPostStepPoint();

	const G4ThreeVector x0 = preStepPoint->GetPosition();
	const G4ThreeVector p0 = aStep.GetDeltaPosition().unit();
	const G4double t0 = preStepPoint->GetGlobalTime();

	G4PhysicsFreeVector* RIndex = aMaterial->GetMaterialPropertiesTable()->GetProperty(kRINDEX);
	if (!RIndex)
		RIndex = aMaterial->GetMaterialPropertiesTable()->GetProperty(kREALRINDEX);

	const G4double charge = aParticle->GetDefinition()->GetPDGCharge();
	const G4double beta = (preStepPoint->GetBeta() + postStepPoint->GetBeta()) * 0.5;

	G4double meanNumberOfPhotons = CalculateAverageNumberOfPhotons(charge, beta, materialID);

	if (meanNumberOfPhotons <= 0.0) {
		//unchanged particle; number of secondaries is 0 anyway after p_particleChange->Initialize(aTrack);
		return p_particleChange;
	}

	meanNumberOfPhotons = meanNumberOfPhotons * aStep.GetStepLength();
	G4int noOfPhotons = (G4int)G4Poisson(meanNumberOfPhotons);

	G4OpticalParameters* optParameters = G4OpticalParameters::Instance();
	//got no idea what's the point of 'optParameters->GetCerenkovStackPhotons()',
	//but staying consistent with G4Cerenkov...
	if (noOfPhotons <= 0 || !optParameters->GetCerenkovStackPhotons())
		return p_particleChange;

	p_particleChange->SetNumberOfSecondaries(noOfPhotons);

	if (optParameters->GetCerenkovTrackSecondariesFirst()) {
		if (aTrack.GetTrackStatus() == fAlive)
			p_particleChange->ProposeTrackStatus(fSuspend);
	}

	G4double lossEnergy = 0.;

	// initializing to keep the compiler silent
	G4double minEnergy = 0.;
	G4double maxEnergy = 0.;
	// or would it be better to keep it in stack and check condition *1* every time in the following 'for'??
	std::vector<std::pair<G4double, G4double>>* bigBetaCDFVector = nullptr;
	if (m_ChRPhysDataVec[materialID].m_aroundBetaValues.front().p_valuesCDF) {
		if (beta > m_ChRPhysDataVec[materialID].m_aroundBetaValues.back().m_betaValue) { // *1*
			bigBetaCDFVector = new std::vector<std::pair<G4double, G4double>>{};
			std::vector<G4ThreeVector>* bigBetaVec = m_ChRPhysDataVec[materialID].p_bigBetaCDFVector;
			bigBetaCDFVector->reserve(bigBetaVec->size());
			G4double maxValueForBigBeta = bigBetaVec->back().getY() - bigBetaVec->back().getZ() / (beta * beta);
			for (auto i = bigBetaVec->begin(); i != bigBetaVec->end() - 1; i++)
				bigBetaCDFVector->emplace_back((*i).getX(), ((*i).getY() - (*i).getZ() / (beta * beta)) / maxValueForBigBeta);
			bigBetaCDFVector->emplace_back(bigBetaVec->back().getX(), 1.);
		}
		else {
			const std::vector<G4ChRPhysTableData::G4AroundBetaBasedValues>& physDataVec = m_ChRPhysDataVec[materialID].m_aroundBetaValues;
			// Returning the higher beta value from tables. With reasonable number of beta steps, some negligible
			// inaccuracies can be expected in the distribution, while I can save some number of processor cycles
			bigBetaCDFVector = physDataVec[static_cast<size_t>(std::lower_bound(physDataVec.begin() + 1, physDataVec.end(), beta,
				[](const G4ChRPhysTableData::G4AroundBetaBasedValues& value1, const G4double value2) {return value1.m_betaValue < value2; }) - physDataVec.begin())].p_valuesCDF;
		}
	}
	else {
		minEnergy = RIndex->Energy(0);
		maxEnergy = RIndex->GetMaxEnergy();
	}

	/*if (bigBetaCDFVector) {
		std::ofstream oVEC;
		std::string fileName = "CDF - ";
		fileName += std::to_string(aTrack.GetKineticEnergy() / MeV) + " - " + std::to_string(beta) + ".csv";
		oVEC.open(fileName, std::ios::out | std::ios::trunc);
		for (const auto& i : *bigBetaCDFVector)
			oVEC << i.first / eV << ',' << i.second << '\n';
		oVEC.close();
	}*/

	for (G4int i = 0; i < noOfPhotons; ++i) {
		G4double rand;
		G4double sampledEnergy, sampledRI;
		G4double cosTheta, sin2Theta;
		// sample an energy
		if (bigBetaCDFVector) {
			do {
				rand = G4UniformRand();
				size_t lowLoc = static_cast<size_t>(std::lower_bound(bigBetaCDFVector->begin() + 1, bigBetaCDFVector->end(), rand,
					[](const std::pair<G4double, G4double>& value1, const G4double value2) {return value1.second < value2; }) - bigBetaCDFVector->begin());
				sampledEnergy = G4LinearInterpolate2D_GetX((*bigBetaCDFVector)[lowLoc].second, (*bigBetaCDFVector)[lowLoc - 1].second,
					(*bigBetaCDFVector)[lowLoc].first, (*bigBetaCDFVector)[lowLoc - 1].first, rand);
				sampledRI = RIndex->Value(sampledEnergy);
				cosTheta = 1. / (sampledRI * beta); //might give > 1. for strange n(E) functions
				sin2Theta = (1.0 - cosTheta) * (1.0 + cosTheta);
			} while (sin2Theta <= 0.);
		}
		else {
			do {
				rand = G4UniformRand();
				sampledEnergy = minEnergy + rand * (maxEnergy - minEnergy);
				sampledRI = RIndex->Value(sampledEnergy);
				cosTheta = 1. / (sampledRI * beta); //might give > 1. for strange n(E) functions
				sin2Theta = (1.0 - cosTheta) * (1.0 + cosTheta);
			} while (sin2Theta <= 0.);
		}
		// Create photon momentum direction vector. The momentum direction is still
		// with respect to the coordinate system where the primary particle
		// direction is aligned with the z axis
		rand = G4UniformRand();
		G4double phi = CLHEP::twopi * rand;
		G4double sinPhi = std::sin(phi);
		G4double cosPhi = std::cos(phi);
		G4double sinTheta = std::sqrt(sin2Theta);

		G4ParticleMomentum photonMomentum(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);

		// Rotate momentum direction back to global reference system
		photonMomentum.rotateUz(p0);

		// Determine polarization of new photon
		G4ThreeVector photonPolarization(cosTheta * cosPhi, cosTheta * sinPhi, -sinTheta);

		// Rotate back to original coordinate system
		photonPolarization.rotateUz(p0);
		// Generate a new photon:
		auto aCerenkovPhoton = new G4DynamicParticle(G4OpticalPhoton::OpticalPhoton(), photonMomentum);

		aCerenkovPhoton->SetPolarization(photonPolarization);
		aCerenkovPhoton->SetKineticEnergy(sampledEnergy);
		lossEnergy += sampledEnergy;

		//The following seems like a very expensive way to change rand distribution
		//However, the idea might be correct for very low energies, i.e.,
		//more photons are emitted from around preStep than around postStep.
		//Any suggestions?
		//=================================================================
		//double beta1 = preStepPoint->GetBeta();
		//double beta2 = postStepPoint->GetBeta();
		//double meanNumberOfPhotons1 = CalculateAverageNumberOfPhotons(charge, beta1, aMaterial);
		//double meanNumberOfPhotons2 = CalculateAverageNumberOfPhotons(charge, beta2, aMaterial);
		//double numberOfPhotons, N;
		//do {
		//	rand = G4UniformRand();
		//	numberOfPhotons = meanNumberOfPhotons1 - rand * (meanNumberOfPhotons1 - meanNumberOfPhotons2);
		//	N = G4UniformRand() * std::max(meanNumberOfPhotons1, meanNumberOfPhotons2);
		//	// Loop checking, 07-Aug-2015, Vladimir Ivanchenko
		//} while (N > numberOfPhotons);
		//=================================================================
		rand = G4UniformRand();
		//More or less, the previous line can replace the commented part...
		//however, the distribution remains uniform no matter what

		G4double delta = rand * aStep.GetStepLength();
		G4double deltaTime = delta / (preStepPoint->GetVelocity() + rand * (postStepPoint->GetVelocity() - preStepPoint->GetVelocity()) * 0.5);

		G4double aSecondaryTime = t0 + deltaTime;
		G4ThreeVector aSecondaryPosition = x0 + rand * aStep.GetDeltaPosition();

		// Generate new G4Track object:
		G4Track* aSecondaryTrack = new G4Track(aCerenkovPhoton, aSecondaryTime, aSecondaryPosition);

		aSecondaryTrack->SetTouchableHandle(aStep.GetPreStepPoint()->GetTouchableHandle());
		aSecondaryTrack->SetParentID(aTrack.GetTrackID());
		p_particleChange->AddSecondary(aSecondaryTrack);
	}

	if (m_useEnergyLoss) {
		//considering only energy loss, but neglecting change in momentum direction...
		//photons have very low energy and are emitted uniformly around the particle so this should be 
		//really meager, and almost seems like IEEE754 error would be greater than the actual change.
		//Or should I maybe consider momentum direction change as well?
		p_particleChange->ProposeEnergy(p_particleChange->GetEnergy() - lossEnergy);
	}
	if (verboseLevel > 1)
		std::cout << "\n Exiting from AlmostOriginalChR_Model::DoIt -- NumberOfSecondaries = "
		<< p_particleChange->GetNumberOfSecondaries() << G4endl;

	if (bigBetaCDFVector && beta > m_ChRPhysDataVec[materialID].m_aroundBetaValues.back().m_betaValue)
		delete bigBetaCDFVector;

	return p_particleChange;
}

G4double G4StandardCherenkovProcess::MinPrimaryEnergy(const G4ParticleDefinition* aParticle, const G4Material* aMaterial) {
	G4double restE = aParticle->GetPDGMass();
	G4AccessPhysicsVector* accessor = reinterpret_cast<G4AccessPhysicsVector*>(aMaterial->GetMaterialPropertiesTable()->GetProperty(kRINDEX));
	if (!accessor)
		accessor = reinterpret_cast<G4AccessPhysicsVector*>(aMaterial->GetMaterialPropertiesTable()->GetProperty(kREALRINDEX));
	if (!accessor) {
		std::ostringstream err;
		err << "Material " << std::quoted(aMaterial->GetName()) << " has no registered RIndex property, i.e., no min energy!\n";
		G4Exception("G4CherenkovProcess::MinPrimaryEnergy", "WE_stdChRProc01", JustWarning, err);
		return -1.;
	}
	G4double maxN = accessor->GetRealDataVectorMax();
	G4double value = restE / std::sqrt(1 - 1 / (maxN * maxN));
	if (verboseLevel > 0)
		std::cout << "For particle " << std::quoted(aParticle->GetParticleName()) << ", minimum energy required\n"
		<< "for producing Cherenkov photons in material " << std::quoted(aMaterial->GetName()) << " is " << value / MeV << " MeV\n";
	return value;
}

void G4StandardCherenkovProcess::BuildPhysicsTable(const G4ParticleDefinition&) {
	std::size_t numOfMaterials = G4Material::GetNumberOfMaterials();
	if (m_ChRPhysDataVec.size() == numOfMaterials)
		return;
	m_ChRPhysDataVec = G4ChRPhysicsTableVector{}; //in case some materials were deleted - rebuilding all physics tables
	m_ChRPhysDataVec.reserve(numOfMaterials);
	for (size_t i = 0; i < numOfMaterials; i++)
		AddExoticRIndexPhysicsTable(i);
	if (verboseLevel > 0)
		PrintChRPhysDataVec();
}

void G4StandardCherenkovProcess::DumpInfo() const {
	std::cout.fill('=');
	std::cout << std::setw(116) << '\n';
	std::cout << "Begin of G4StandardCherenkovProcess::DumpInfo()\n"
		"\"G4StandardCherenkovProcess\" is a Cherenkov radiation process that's based on the original Frank-Tamm theory.\n"
		"That means Cherenkov photons are emitted along the lateral surface of the cone relative to a charged\n"
		"particle that passes through the material. The cone angle can be expressed as:\n"
		"cos(ThetaChR) = 1 / (beta * RIndex).\n"
		"To read more about the first Frank-Tamm theory, see:\n"
		"I.M.Frank, I.E.Tamm, Coherent visible radiation of fast electrons passing through matter,\n"
		"Dokl. Acad. Sci. USSR 14 (1937) 109-114\n\n"
		"NOTE1: this model currently supports only optical photons and does not generate photons in the X-ray\n"
		"region. On the other hand, the process removes all the limitations that exist in the G4Cerenkov\n"
		"class, meaning that the model can consider any kind of refractive index dependencies.\n\n"
		"NOTE2: this process should generate good results as long as the considered radiator can be approximated\n"
		"as \"ideal\", i.e., the Frank-Tamm theory is written for an infinitely thick emitter. If thin radiators\n"
		"are considered, one should consider other models. For more information, see the G4CherenkovProcess class\n\n"
		"If you want to read about what Cherenkov process is, use \"G4StandardCherenkovProcess::ProcessDescription()\" method\n"
		"End of G4StandardCherenkovProcess::DumpInfo()\n"
		<< std::setw(116) << '\n' << std::endl;
}

void G4StandardCherenkovProcess::ProcessDescription(std::ostream& outStream) const {
	outStream.fill('=');
	outStream << std::setw(116) << '\n';
	outStream << "Begin of G4StandardCherenkovProcess::ProcessDescription():\n\n"
		"The Cherenkov radiation was discovered in 1934 by S.I. Vavilov and P.A. Cherenkov (Vavilov's student).\n"
		"The first theoretical explanation was provided in 1937 by I.M. Frank and I.E. Tamm. According to our\n"
		"understanding, when a charged particle moves through a medium faster than the phase velocity of light,\n"
		"photons of a wide energy range (predominantly in the optical region) are emitted. \"Moves faster\" means\n"
		"the condition \"beta * n >= 1\", where \"beta\" is a relativistic reduced velocity (= v / c) and \"n\" is\n"
		"the refractive index, is satisfied.\n\n"
		"In order to use this class in Geant4, one must define the refractive index of a material through the class\n"
		"G4MaterialPropertiesTable. To do so, one needs to provide a corresponding key index that can be found in\n"
		"\"G4MaterialPropertiesIndex.hh\" (just remove \"k\" from the enum). Also, note that it's advisable to\n"
		"define absorption always; otherwise an optical photon might enter an infinite loop of total internal\n"
		"reflections.\n\n"
		"To see more details about this specific C++ class, use the G4StandardCherenkovProcess::DumpInfo() method.\n\n"
		"Some English literature I know of that one might find interesting and is a nice summary of everything:\n"
		"1. J.V. Jelley, Cherenkov radiation and its Applications, Pergamon Press, New York, 1958\n"
		"2. B.M. Bolotovskii \"Vavilov-Cherenkov radiation: its discovery and application\" Phys. Usp. 52(11), (2009) 1099-1110\n"
		"3. A.P. Kobzev \"On the radiation mechanism of a uniformly moving charge\", Phys. Part. Nucl. 45(3), (2014) 628-653\n\n"
		"End of G4StandardCherenkovProcess::ProcessDescription()\n"
		<< std::setw(116) << '\n' << std::endl;
}

void G4StandardCherenkovProcess::PrintChRPhysDataVec(const unsigned char printLevel, const G4Material* aMaterial) {
	const G4MaterialTable* theMaterialTable = G4Material::GetMaterialTable();
	std::cout.fill('=');
	std::cout << std::setw(66) << '\n';
	if (aMaterial)
		std::cout << "Begin of PrintChRPhysDataVec( " << std::to_string(printLevel) << ", " << aMaterial->GetName() << " )\n\n";
	else
		std::cout << "Begin of PrintChRPhysDataVec( " << std::to_string(printLevel) << ", nullptr )\n\n";

	std::cout << "Number of emitted Cherenkov photons can be calculated as:\nconst * (leftIntegral - rightIntegral / beta^2)\n";
	if (aMaterial) { //if a specific material is selected, the method prints only about that method
		if (printLevel == 0) {
			PrintSimpleTables(aMaterial, m_ChRPhysDataVec[aMaterial->GetIndex()].m_aroundBetaValues);
			goto ExitPrintFunction;
		}
		PrintMoreComplexTables(printLevel, aMaterial, m_ChRPhysDataVec[aMaterial->GetIndex()]);
		goto ExitPrintFunction;
	}
	std::cout << "\nBuilt tables of G4StandardCherenkovProcess are:\n\n";
	if (printLevel == 0) {
		for (size_t i = 0; i < m_ChRPhysDataVec.size(); i++)
			PrintSimpleTables((*theMaterialTable)[i], m_ChRPhysDataVec[i].m_aroundBetaValues);
		goto ExitPrintFunction;
	}
	std::cout << std::setfill('+') << std::setw(66) << '\n';
	for (size_t i = 0; i < m_ChRPhysDataVec.size(); i++) {
		PrintMoreComplexTables(printLevel, (*theMaterialTable)[i], m_ChRPhysDataVec[i]);
		std::cout << '\n' << std::setfill('+') << std::setw(66) << '\n';
	}
ExitPrintFunction:
	if (aMaterial)
		std::cout << "\nEnd of PrintChRPhysDataVec( " << std::to_string(printLevel) << ", " << aMaterial->GetName() << " )\n";
	else
		std::cout << "\nEnd of PrintChRPhysDataVec( " << std::to_string(printLevel) << ", nullptr )\n";
	std::cout << std::setfill('=') << std::setw(66) << '\n';
}

//=========protected G4CherenkovProcess:: methods=========

G4double G4StandardCherenkovProcess::CalculateAverageNumberOfPhotons(const G4double aCharge, const G4double betaValue, const size_t materialID) {
	constexpr G4double Rfact = 369.81 / (eV * cm);
	if (betaValue <= 0)
		return 0.;
	const std::vector<G4ChRPhysTableData::G4AroundBetaBasedValues>& physDataVec = m_ChRPhysDataVec[materialID].m_aroundBetaValues;
	// the following condition should never happen - it was already done in the StepLength method
	/*if (physDataVec.size() <= 1)
		return 0.;*/
	G4double deltaE, ChRRightIntPart;
	if (betaValue <= physDataVec.front().m_betaValue) {
		deltaE = 0.;
		ChRRightIntPart = 0.;
	}
	else if (betaValue >= physDataVec.back().m_betaValue) {
		deltaE = physDataVec.back().m_leftIntegralValue;
		ChRRightIntPart = physDataVec.back().m_rightIntegralValue;
	}
	else {
		size_t lowLoc = static_cast<size_t>(std::lower_bound(physDataVec.begin() + 1, physDataVec.end(), betaValue,
			[](const G4ChRPhysTableData::G4AroundBetaBasedValues& value1, const G4double value2) {return value1.m_betaValue < value2; }) - physDataVec.begin());
		deltaE = G4LinearInterpolate2D_GetY(physDataVec[lowLoc].m_leftIntegralValue, physDataVec[lowLoc - 1].m_leftIntegralValue,
			physDataVec[lowLoc].m_betaValue, physDataVec[lowLoc - 1].m_betaValue, betaValue);
		ChRRightIntPart = G4LinearInterpolate2D_GetY(physDataVec[lowLoc].m_rightIntegralValue, physDataVec[lowLoc - 1].m_rightIntegralValue,
			physDataVec[lowLoc].m_betaValue, physDataVec[lowLoc - 1].m_betaValue, betaValue);
	}
	return Rfact * pow2(aCharge) / pow2(eplus) * (deltaE - ChRRightIntPart / pow2(betaValue));
}

//=========private G4StandardCherenkovProcess:: methods=========
G4bool G4StandardCherenkovProcess::AddExoticRIndexPhysicsTable(const size_t materialID, G4bool forceExoticFlag) {
	const G4MaterialTable* theMaterialTable = G4Material::GetMaterialTable();
	G4ChRPhysTableData thePhysVecData{};
	G4MaterialPropertiesTable* MPT = (*theMaterialTable)[materialID]->GetMaterialPropertiesTable();
	if (!MPT) {
		m_ChRPhysDataVec.push_back(std::move(thePhysVecData));
		return false;
	}
	G4AccessPhysicsVector* RIndex = reinterpret_cast<G4AccessPhysicsVector*>(MPT->GetProperty(kRINDEX));
	if (!RIndex) //NOTE: I'm not sure if those are left separate on purpose!
		RIndex = reinterpret_cast<G4AccessPhysicsVector*>(MPT->GetProperty(kREALRINDEX));
	if (!RIndex) {
		m_ChRPhysDataVec.push_back(std::move(thePhysVecData));
		return false;
	}
	const std::vector<G4double>& RIVector = RIndex->GetDataVector();
	G4double nMax = RIndex->GetRealDataVectorMax();
	G4double nMin = RIndex->GetRealDataVectorMin();
	if (nMax <= 1.) {
		m_ChRPhysDataVec.push_back(std::move(thePhysVecData));
		return false;
	}
	const std::vector<G4double>& energyVec = RIndex->GetBinVector();
	G4double betaLowLimit = 1 / nMax;
	G4double betaHighLimit = 1 / nMin;
	// for the following, the radiation can either be produced, or not
	if (nMax == nMin) {
		G4double deltaE = energyVec.back() - energyVec.front();
		G4double ChRIntensity = deltaE / nMax;
		thePhysVecData.m_aroundBetaValues.reserve(2);
		thePhysVecData.m_aroundBetaValues.emplace_back(betaLowLimit, deltaE, ChRIntensity);
		thePhysVecData.m_aroundBetaValues.emplace_back(betaLowLimit, deltaE, ChRIntensity);
		m_ChRPhysDataVec.push_back(std::move(thePhysVecData));
		return false;
	}
	if (!forceExoticFlag) {
		const G4Material* aMaterial = (*theMaterialTable)[materialID];
		G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
		for (const auto* i : *lvStore) {
			// comparing material memory addresses
			if (i->GetMaterial() == aMaterial && !std::is_sorted(RIndex->GetDataVector().begin(), RIndex->GetDataVector().end())) {
				forceExoticFlag = true;
				break;
			}
		}
	}
	G4double deltaBeta = (betaHighLimit - betaLowLimit) / m_noOfBetaSteps;
	thePhysVecData.m_aroundBetaValues.reserve((size_t)m_noOfBetaSteps + 1);
	for (size_t j = 0; j <= m_noOfBetaSteps; j++) {
		G4double deltaE = 0.;
		G4double ChRIntensity = 0.;
		G4double beta = betaLowLimit + (G4double)j * deltaBeta;
		std::vector<std::pair<G4double, G4double>>* aCDFValues = nullptr;
		std::vector<G4ThreeVector>* bigBetaVector = nullptr;
		if (forceExoticFlag) {
			aCDFValues = new std::vector<std::pair<G4double, G4double>>{};
			aCDFValues->reserve(energyVec.size());
			aCDFValues->emplace_back(energyVec.front(), 0.);
		}
		if (j == m_noOfBetaSteps && forceExoticFlag) {
			bigBetaVector = new std::vector<G4ThreeVector>{};
			bigBetaVector->reserve(energyVec.size());
			bigBetaVector->emplace_back(energyVec.front(), 0., 0.);
		}
		for (size_t k = 1; k < RIVector.size(); k++) {
			//the following two are for the ChR condition
			G4double preTemp = beta * RIVector[k - 1];
			G4double postTemp = beta * RIVector[k];
			if (preTemp >= 1. && postTemp >= 1.) {
				ChRIntensity += (energyVec[k] - energyVec[k - 1]) * 0.5 *
					(1 / pow2(RIVector[k]) + 1 / pow2(RIVector[k - 1]));
				deltaE += energyVec[k] - energyVec[k - 1];
			}
			else if (preTemp < 1. && postTemp >= 1.) {
				//boundary condition: RIndex = 1 / beta
				G4double tempE = G4LinearInterpolate2D_GetX(RIVector[k - 1], RIVector[k], energyVec[k - 1], energyVec[k], 1 / beta);
				ChRIntensity += (energyVec[k] - tempE) * 0.5 *
					(1 / (pow2(1 / beta)) + 1 / (pow2(RIVector[k])));
				deltaE += energyVec[k] - tempE;
			}
			else if (preTemp >= 1. /*&& postTemp < 1.*/) {
				//boundary condition: RIndex = 1 / beta
				G4double tempE = G4LinearInterpolate2D_GetX(RIVector[k - 1], RIVector[k], energyVec[k - 1], energyVec[k], 1 / beta);
				ChRIntensity += (tempE - energyVec[k - 1]) * 0.5 *
					(1 / pow2(1 / beta) + 1 / pow2(RIVector[k - 1]));
				deltaE += tempE - energyVec[k - 1];
			}
			if (aCDFValues)
				aCDFValues->emplace_back(energyVec[k], deltaE - ChRIntensity / pow2(beta));
			if (bigBetaVector)
				bigBetaVector->emplace_back(energyVec[k], deltaE, ChRIntensity);
		}
		thePhysVecData.m_aroundBetaValues.emplace_back(beta, deltaE, ChRIntensity);
		if (aCDFValues) {
			// first normalize to [0., 1.] and then pass the pointer
			for (auto& i : *aCDFValues) {
				if (aCDFValues->back().second != 0) // just for the betaMin vector
					i.second /= aCDFValues->back().second;
			}
			thePhysVecData.m_aroundBetaValues.back().p_valuesCDF = aCDFValues;
		}
		if (bigBetaVector)
			thePhysVecData.p_bigBetaCDFVector = bigBetaVector;
	}
	if (m_ChRPhysDataVec.size() == materialID)
		m_ChRPhysDataVec.push_back(std::move(thePhysVecData));
	else
		m_ChRPhysDataVec[materialID] = std::move(thePhysVecData);
	return true;
}

void G4StandardCherenkovProcess::RemoveExoticRIndexPhysicsTable(const size_t materialID) {
	try {
		// error conditions should never happen... anyway, we are in G4State_Idle, so a few processor cycles won't hurt
		G4ChRPhysTableData& thePhysVecData = m_ChRPhysDataVec.at(materialID);

		delete thePhysVecData.p_bigBetaCDFVector;
		thePhysVecData.p_bigBetaCDFVector = nullptr;

		for (auto& i : thePhysVecData.m_aroundBetaValues) {
			if (!i.p_valuesCDF) {
				std::string err{ "No ChR exotic physics table data found while the m_exoticRIndex flag is 'true'!\n" };
				err += "This is a problem in the logic of the code, please report the issue!\n";
				throw err;
			}
			delete i.p_valuesCDF;
			i.p_valuesCDF = nullptr;
		}
	}
	catch (std::out_of_range) {
		std::ostringstream err;
		err << "The material with index " << materialID << " is not found in the built physics tables!\n";
		G4Exception("G4StandardCherenkovProcess::RemoveExoticRIndexPhysicsTable", "FE_stdChRProc02", FatalException, err);
	}
	catch (const std::string err) {
		G4Exception("G4StandardCherenkovProcess::RemoveExoticRIndexPhysicsTable", "FE_stdChRProc03", FatalException, err.c_str());

	}
}

//=========static methods of translation unit=========

static void PrintSimpleTables(const G4Material* aMaterial, const std::vector<G4ChRPhysTableData::G4AroundBetaBasedValues>& theCurrentData) {
	const size_t tableSize = theCurrentData.size();
	std::cout << std::right << std::setfill('_') << std::setw(52) << '\n'
		<< "| Material: " << std::setfill(' ') << std::setw(40) << "|\n"
		<< "| " << std::setw(48) << std::left << aMaterial->GetName() << "|\n"
		<< '|' << std::right << std::setfill('_') << std::setw(51) << "|\n";
	if (tableSize <= 1) {
		std::cout << "| No physics table has been built for this" << std::setfill(' ') << std::setw(10) << "|\n"
			<< "| material (n = 1 or no n(E))" << std::setw(23) << "|\n"
			<< '|' << std::right << std::setfill('_') << std::setw(51) << "|\n";
		return;
	}
	std::cout << "|  Beta  | Left ChR integral | Right ChR integral |\n"
		<< '|' << std::setw(9) << '|' << std::setw(20) << '|' << std::setw(22) << "|\n";
	std::cout.fill(' ');
	for (size_t i = 0; i < tableSize; i++) {
		std::cout << '|' << std::setprecision(4) << std::fixed << std::setw(7) << theCurrentData[i].m_betaValue << " |"
			<< std::scientific << std::setw(15) << theCurrentData[i].m_leftIntegralValue << std::setw(5) << '|'
			<< std::setw(15) << theCurrentData[i].m_rightIntegralValue << std::setw(7) << "|\n";
	}
	std::cout << '|' << std::setfill('_') << std::setw(9) << '|' << std::setw(20) << '|' << std::setw(22) << "|\n";
}

static void PrintMoreComplexTables(const unsigned char printLevel, const G4Material* aMaterial, const G4ChRPhysTableData& theCurrentData) {
	const size_t tableSize = theCurrentData.m_aroundBetaValues.size();
	if (tableSize <= 1) {
		std::cout << "No G4StandardCherenkovProcess physics tables were found for material "
			<< std::quoted(aMaterial->GetName()) << "\nThat means the refractive index of the material is '1', or it is not defined!\n";
		return;
	}
	if (!theCurrentData.m_aroundBetaValues.front().p_valuesCDF) {
		std::cout << "Material " << std::quoted(aMaterial->GetName()) << " has only standard G4StandardCherenkovProcess\n"
			<< "physics tables built (no physics tables for exotic refractive indices were found):\n";
		PrintSimpleTables(aMaterial, theCurrentData.m_aroundBetaValues);
		return;
	}
	std::cout << "Material: " << std::quoted(aMaterial->GetName())
		<< "\nG4StandardCherenkovProcess physics tables for exotic refractive indices found!\n"
		<< "Standard physics tables are:\n";
	PrintSimpleTables(aMaterial, theCurrentData.m_aroundBetaValues);
	std::cout << "\nCDF tables between betaMin and betaMax are:\n";
	size_t noOfColumns = 8; // going with 8 max possible no of columns per table
	size_t remainder = tableSize % noOfColumns; // no of elements not in tables
	size_t noOfTables = tableSize / noOfColumns; // no of table parts
	if (remainder != 0) // if I can't place all elements in tables, no of tables ++
		noOfTables++;
	noOfColumns = tableSize / noOfTables; // normal number of columns per tables
	remainder = tableSize % noOfColumns; // no of tables with an extra column
	auto nextBeginIter = theCurrentData.m_aroundBetaValues.begin();
	size_t noOfRIndexValues = theCurrentData.m_aroundBetaValues.front().p_valuesCDF->size();
	std::cout.fill('_');
	for (size_t i = 0; i < noOfTables; i++) {
		std::vector<G4ChRPhysTableData::G4AroundBetaBasedValues>::const_iterator progressiveIter;
		size_t nextEnd = noOfColumns;
		if (remainder != 0) {
			remainder--;
			nextEnd++;
		}
		//following setw(NUMBER), where NUMBER = nextEnd * " | x.xxxx" + "| x.xxx" + " |\n" = nextEnd * 9 + 10
		std::cout << std::setw(nextEnd * 9 + 10) << '\n';
		std::cout.fill(' ');
		std::cout << "|  ChR  |" << std::right
			<< std::setw((nextEnd * 9 + 1) / 2 - 2) << "beta" << std::setw(nextEnd * 9 - ((nextEnd * 9 + 1) / 2 - 3)) << "|\n";
		std::cout << "| photon|" << std::setfill('-') << std::setw(nextEnd * 9 + 1) << "|\n";
		std::cout << "| E [eV]|";
		progressiveIter = nextBeginIter;
		std::cout.precision(4);
		for (; progressiveIter != nextBeginIter + nextEnd; progressiveIter++)
			std::cout << ' ' << std::fixed << progressiveIter->m_betaValue << " |";
		std::cout << "\n|" << std::setfill('_') << std::setw(8) << '|';
		for (size_t k = 0; k < nextEnd; k++)
			std::cout << std::setw(9) << '|';
		std::cout << '\n';
		for (size_t j = 0; j < noOfRIndexValues; j++) {
			progressiveIter = nextBeginIter;
			std::cout << "| " << std::setprecision(3) << (*theCurrentData.m_aroundBetaValues.front().p_valuesCDF)[j].first / eV;
			std::cout.precision(4);
			for (; progressiveIter != nextBeginIter + nextEnd; progressiveIter++)
				std::cout << " | " << (*(*progressiveIter).p_valuesCDF)[j].second;
			std::cout << " |\n";
		}
		nextBeginIter = progressiveIter;
		std::cout << '|' << std::setw(8) << "|";
		for (size_t k = 0; k < nextEnd; k++)
			std::cout << std::setw(9) << '|';
		std::cout << std::endl;
	}
	if (printLevel < 2)
		return;
	std::cout << "\nCDF for higher than betaMax is calculated based on the following values:\n";
	std::cout << std::setfill('_') << std::setw(52) << '\n';
	std::cout << "| E [eV] | Left ChR integral | Right ChR integral |\n"
		<< '|' << std::setw(9) << '|' << std::setw(20) << '|' << std::setw(22) << "|\n";
	std::cout.fill(' ');
	for (const auto& i : *theCurrentData.p_bigBetaCDFVector) {
		std::cout << '|' << std::setprecision(4) << std::fixed << std::setw(7) << i.getX() / eV << " |"
			<< std::scientific << std::setw(15) << i.getY() << std::setw(5) << '|'
			<< std::setw(15) << i.getZ() << std::setw(7) << "|\n";
	}
	std::cout << '|' << std::setfill('_') << std::setw(9) << '|' << std::setw(20) << '|' << std::setw(22) << "|\n";
}