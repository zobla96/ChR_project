//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//G4 headers
#include "G4ThinTargetChR_Model.hh"
#include "G4ParticleChange.hh"
#include "G4Poisson.hh"
#include "G4OpticalPhoton.hh"
#include "G4ExtraOpticalParameters.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4UIcmdWithADouble.hh"
#include "SomeGlobalNamespace.hh"

//=========public G4ThinTargetChR_ModelMessenger:: methods=========

G4ThinTargetChR_ModelMessenger::G4ThinTargetChR_ModelMessenger(G4ThinTargetChR_Model* anObject)
: p_thinChRTarget(anObject) {
	p_multiplierCoef = new G4UIcmdWithADouble("/process/optical/G4ChRProcess/Models/ThinChRModelMultiplier", this);
	p_multiplierCoef->SetGuidance("Used to change K in equation for Cherenkov cone widening, i.e.:");
	p_multiplierCoef->SetGuidance("K * lambda / (RIndex * thickness * sinTheta)");
	p_multiplierCoef->SetGuidance("The K is currently adapted according to experimental data (0.35), but it might");
	p_multiplierCoef->SetGuidance("be needed to adjust it in the future. I left it as an UIcmd for easier adjusting.");
	p_multiplierCoef->SetGuidance("Still, don't use this command unless you have some good reason to!");
	p_multiplierCoef->SetParameterName("valueK", false);
	p_multiplierCoef->SetRange("valueK>0.");
	p_multiplierCoef->SetToBeBroadcasted(true);
	p_multiplierCoef->AvailableForStates(G4State_Idle);
}

G4ThinTargetChR_ModelMessenger::~G4ThinTargetChR_ModelMessenger() {
	delete p_multiplierCoef;
}

void G4ThinTargetChR_ModelMessenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr) {
	if (uiCmd == p_multiplierCoef) {
		p_thinChRTarget->SetMultiplierCoef(p_multiplierCoef->ConvertToDouble(aStr));
	}
}

//=========public G4ThinTargetChR_Model:: methods=========

G4ThinTargetChR_Model::G4ThinTargetChR_Model(const char* theName)
: G4BaseChR_Model(theName), m_coef(0.3485) {
	m_theMessenger = std::make_unique<G4ThinTargetChR_ModelMessenger>(this);
	m_includeFiniteThickness = true;
}

G4VParticleChange* G4ThinTargetChR_Model::PostStepModelDoIt(const G4Track& aTrack, const G4Step& aStep, const G4CherenkovMatData& aChRMatData) {
	p_particleChange->Initialize(aTrack);
	const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
	const G4Material* aMaterial = aTrack.GetMaterial();
	size_t materialID = aMaterial->GetIndex();

	//NOTE: no need for various checking, bcs the PostStepIntLNG would kill it
	//already if something was off

	const G4double matThickness = aChRMatData.m_matThickness;
	if (matThickness <= 0.) {
		const char* err = "You are trying to use a Cherenkov model for thin targets without specifying the thickness!\n";
		G4Exception("G4ThinTargetChR_Model::PostStepModelDoIt", "FE_ThinChR01", FatalException, err);
	}

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
	//got no idea what's the point of 'optParameters->GetCerenkovStackPhotons()', but staying consistent...
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
	if (aChRMatData.m_exoticRIndex) {
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
		if (aChRMatData.m_exoticRIndex) {
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
			while (true) {
				rand = G4UniformRand();
				sampledEnergy = minEnergy + rand * (maxEnergy - minEnergy);
				sampledRI = RIndex->Value(sampledEnergy);
				cosTheta = 1. / (sampledRI * beta); //might give > 1. for strange n(E) functions
				sin2Theta = (1.0 - cosTheta) * (1.0 + cosTheta);
				if (sin2Theta <= 0.) {
					//the following 'if' is to prevent bad distribution if the user modified an almost non-exotic RIndex
					if (!aChRMatData.m_exoticFlagInital)
						minEnergy = sampledEnergy;
					continue;
				}
				break;
			}
		}
		//h * c = 1.239841984e-6 * m * eV
		G4double waveLng = 1.239841984e-6 * m * eV / sampledEnergy;
		G4double sinTheta = std::sqrt(sin2Theta);

		G4double gaussSigma = m_coef * waveLng / (sampledRI * matThickness  * sinTheta);
		G4double thetaChR = std::acos(cosTheta);

		thetaChR = G4RandGauss::shoot(thetaChR, gaussSigma);
		cosTheta = std::cos(thetaChR);
		sinTheta = std::sin(thetaChR);
		//change here
		rand = G4UniformRand();
		G4double phi = CLHEP::twopi * rand;
		G4double sinPhi = std::sin(phi);
		G4double cosPhi = std::cos(phi);

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

		rand = G4UniformRand();

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

	if (m_useModelWithEnergyLoss) {
		//considering only energy loss, but neglecting change in momentum direction...
		//photons have very low energy and are emitted uniformly around the particle so this should be 
		//really meager, and almost seems like IEEE754 error would be greater than the actual change.
		//Or should I maybe consider momentum direction change as well?
		p_particleChange->ProposeEnergy(p_particleChange->GetEnergy() - lossEnergy);
	}
	if (m_verboseLevel > 1)
		std::cout << "\n Exiting from G4ThinTargetChR_Model::PostStepModelDoIt -- NumberOfSecondaries = "
		<< p_particleChange->GetNumberOfSecondaries() << G4endl;

	if (bigBetaCDFVector && beta > m_ChRPhysDataVec[materialID].m_aroundBetaValues.back().m_betaValue)
		delete bigBetaCDFVector;

	return p_particleChange;
}

void G4ThinTargetChR_Model::DumpModelInfo() const {
	std::cout << "The \"G4ThinTargetChR_Model\" is another model of Cherenkov radiation that allows us to\n"
		<< "imagine how thin radiators affect Cherenkov radiation angular distribution. Due to the limited\n"
		<< "size of such radiators, Cherenkov radiation interference effects cannot occur, which finally\n"
		<< "leads to the change of the thickness of the cone's lateral surface from a delta function into\n"
		<< "a normal distribution. It can be expressed as (see reference 1 below):\n"
		<< "deltaThetaFWHM = 2.78 * lambda / (Pi * n * trajectoryLength * sin(theta)),\n"
		<< "where lambda is a specific wavelength, trajectoryLength can be considered as a radiator thickness\n"
		<< "for thin radiators, theta is the angle of Cherenkov cone, and n is the refractive index of the material\n"
		<< "for the given wavelength. As it is a normal distribution, we can write a variable K as:\n"
		<< "K = 2.78 / (Pi * 2.355) ~ 0.38. Still, this model was introduced and explained in reference 2 (see\n"
		<< "below), where the variable K was reduced to the value of 0.3485.\nAlso see:\n"
		<< "1. R.L.Mather, Cherenkov radiation from protons and measurement of proton velocity and kinetic\n"
		<< "energy, Phys. Rev. 84(2) (1951) 181–190; DOI: 10.1103/PhysRev.84.181\n"
		<< "2. B.Djurnic, A.Potylitsyn, A.Bogdanov, S.Gogolev, On the rework and development of new Geant4\n"
		<< "Cherenkov models, (in press); DOI: (in press)\n\n"
		<< "NOTE1: this model currently supports only optical photons and does not generate photons in the X - ray\n"
		<< "region. On the other hand, the base class \"G4BaseChR_Model\" removes all the limitations that exist in\n"
		<< "the G4Cerenkov class, meaning that the model can consider any kind of refractive index dependencies.\n\n"
		<< "NOTE2: this model can consider a charged particle that crosses a radiator of finite thickness\n"
		<< "and infinite transverse sizes (it moves almost perpendicular relative to transverse sizes)\n";
}