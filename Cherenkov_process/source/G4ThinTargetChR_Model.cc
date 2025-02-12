//##########################################
//#######        VERSION 1.0.0       #######
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
#include "G4TransportationManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4AffineTransform.hh"
#include "G4MTRunManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"

//static helper functions based around this translation unit - definition at the end of the file
using dimBufferIter_t = std::vector<G4ThreeVector>::iterator;
[[nodiscard]] static unsigned char HandleDimensionBufferVector(const dimBufferIter_t begin, const dimBufferIter_t end, const G4Box* aBoxSolid,
	const unsigned char passNo, std::array<size_t, 3>& dimensionNoOfLayers);
static void PrintJustWarningExplanations(const size_t id, const void* aType1 = nullptr, const void* aType2 = nullptr);
static void PrintSuccessInfo(const G4LogicalVolume* aLogic, const G4CherenkovMatData& matData, std::array<size_t, 3>* noLayersPerDimension = nullptr);

//=========public G4ThinTargetChR_Model:: methods=========

G4ThinTargetChR_Model::G4ThinTargetChR_Model(const char* theName)
: G4BaseChR_Model(theName) {
	m_includeFiniteThickness = true;
}

G4VParticleChange* G4ThinTargetChR_Model::PostStepModelDoIt(const G4Track& aTrack, const G4Step& aStep, const G4CherenkovMatData& aChRMatData) {
	p_particleChange->Initialize(aTrack);

	const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
	const G4Material* aMaterial = aTrack.GetMaterial();
	size_t materialID = aMaterial->GetIndex();

	// No need for various checking, bcs the PostStepIntLNG
	// would kill it already if something was off
	if (aChRMatData.m_minAxis > 2) {
		const char* err = "Currently, G4ThinTargetChR_Model can be used only for G4Tubs and G4Box\n"
			"volumes and only the thickness (a single dimension) should be\n"
			"small (e.g., '1 mm' and less). If you want to produce Cherenkov\n"
			"radiation in thicker targets, use standard models for better\n"
			"for better performance (results won't differ).\n"
			"If, however, you need more complex shapes, currently, this model\n"
			"cannot consider them correctly.\n"
			"Note that only fully filled G4Tubs is supported!\n";
		G4Exception("G4ThinTargetChR_Model::PostStepModelDoIt", "FE_ThinChR01", FatalException, err);
	}

	const G4StepPoint* preStepPoint = aStep.GetPreStepPoint();
	const G4StepPoint* postStepPoint = aStep.GetPostStepPoint();

	const G4ThreeVector x0 = preStepPoint->GetPosition();
	const G4ThreeVector p0 = aStep.GetDeltaPosition().unit();
	const G4double t0 = preStepPoint->GetGlobalTime();

	const G4VTouchable* aTouchable = aTrack.GetTouchable();
	// Moving to local coordinate system to easily find all points.
	G4AffineTransform aTransform{ aTouchable->GetRotation(), aTouchable->GetTranslation() }; // local to global
	G4AffineTransform inverseTransform{ aTransform.Inverse() }; // global to local
	G4ThreeVector directionInLocal{ p0 };
	inverseTransform.ApplyAxisTransform(directionInLocal);
	G4ThreeVector prePositionInLocal{ x0 };
	inverseTransform.ApplyPointTransform(prePositionInLocal);
	G4ThreeVector aLocalMiddlePoint{ 0., 0., 0. };
	if (aChRMatData.p_middlePoint) {
		aLocalMiddlePoint = *aChRMatData.p_middlePoint;
		inverseTransform.ApplyPointTransform(aLocalMiddlePoint);
	}
	
	G4ThreeVector localEntryPoint, localExitPoint; // extended x0 and p0 to find entry and exit points in local
	if (!FindParticleEntryAndExitPoints(localEntryPoint, localExitPoint, aLocalMiddlePoint, directionInLocal, prePositionInLocal, aChRMatData))
		return p_particleChange;

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
	//got no idea what's the point of 'optParameters->GetCerenkovStackPhotons()', but staying consistent with G4Cerenkov
	if (noOfPhotons <= 0 || !optParameters->GetCerenkovStackPhotons())
		return p_particleChange;

	p_particleChange->SetNumberOfSecondaries(noOfPhotons);

	if (optParameters->GetCerenkovTrackSecondariesFirst())
		if (aTrack.GetTrackStatus() == fAlive)
			p_particleChange->ProposeTrackStatus(fSuspend);

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

	G4double theCosAngle;
	if (aChRMatData.m_minAxis == 0)
		theCosAngle = std::abs(directionInLocal.dot({ 1., 0., 0. }));
	else if (aChRMatData.m_minAxis == 1)
		theCosAngle = std::abs(directionInLocal.dot({ 0., 1., 0. }));
	else /*aChRMatData.m_minAxis == 2*/
		theCosAngle = std::abs(directionInLocal.dot({ 0., 0., 1. }));

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

		/*
		the following gaussSigma is from the previous versions (0.5 and earlier)
		m_coef = 0.38 ~ 2.78 / (pi * 2.355)
		later adapted to: m_coef = 0.3485
		G4double gaussSigma = m_coef * waveLng / (sampledRI * matThickness  * sinTheta);
		*/
		//The following equation is another possibility to express the Gauss sigma - another theory, but should be yet considered
		/*G4double gaussSigma = 1.18 * waveLng * beta * std::cos(psi) * (1 + beta * std::sin(thetaChR) * std::sin(psi))
			/ (CLHEP::pi * matThickness * (beta * std::sin(thetaChR) + std::sin(psi)));*/
		G4double thetaChR = std::acos(cosTheta);
		
		// first obtain phi because gaussSigma depends on the angle of emission, that's if
		// the radiator is rotated relative to the charged particle
		rand = G4UniformRand();
		G4double phi = CLHEP::twopi * rand;
		G4double sinPhi = std::sin(phi);
		G4double cosPhi = std::cos(phi);

		G4ThreeVector photonMomentum{ sinTheta * cosPhi, sinTheta * sinPhi, cosTheta };
		// Now to global - just a quick solution and might change in the future to have a single transform matrix (this is a waste of processor cycles)
		photonMomentum.rotateUz(p0);
		// And then to local
		inverseTransform.ApplyAxisTransform(photonMomentum);
		
		if (G4double neededDistance = CalculateGaussSigmaDistance(localEntryPoint, localExitPoint, photonMomentum, aChRMatData);
			neededDistance != DBL_MAX) {
			G4double gaussSigma = 0.42466 * waveLng * theCosAngle / (sampledRI * neededDistance);
			thetaChR = G4RandGauss::shoot(thetaChR, gaussSigma);
			cosTheta = std::cos(thetaChR);
			sinTheta = std::sin(thetaChR);
		}

		photonMomentum.set(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);

		// Rotate momentum direction back to global reference system
		photonMomentum.rotateUz(p0);

		// Determine polarization of new photon
		G4ThreeVector photonPolarization{ cosTheta * cosPhi, cosTheta * sinPhi, -sinTheta };

		// Rotate back to original coordinate system
		photonPolarization.rotateUz(p0);
		// Generate a new photon:
		auto aCerenkovPhoton = new G4DynamicParticle{ G4OpticalPhoton::OpticalPhoton(), photonMomentum };

		aCerenkovPhoton->SetPolarization(photonPolarization);
		aCerenkovPhoton->SetKineticEnergy(sampledEnergy);
		lossEnergy += sampledEnergy;

		rand = G4UniformRand();

		G4double delta = rand * aStep.GetStepLength();
		G4double deltaTime = delta / (preStepPoint->GetVelocity() + rand * (postStepPoint->GetVelocity() - preStepPoint->GetVelocity()) * 0.5);

		G4double aSecondaryTime = t0 + deltaTime;
		G4ThreeVector aSecondaryPosition{ x0 + rand * aStep.GetDeltaPosition() };

		// Generate new G4Track object:
		G4Track* aSecondaryTrack = new G4Track{ aCerenkovPhoton, aSecondaryTime, aSecondaryPosition };

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
		"simulate how thin radiators affect Cherenkov radiation angular distribution. Due to the limited\n"
		"size of such radiators, Cherenkov radiation interference effects cannot occur, which finally\n"
		"leads to the change of the thickness of the cone's lateral surface from a delta function into\n"
		"a normal distribution. It can be expressed as (see the reference below):\n"
		"deltaThetaFWHM = lambda * aCosAngle / (n * radiatorThickness * someDistance),\n"
		"where lambda is a specific wavelength, aCosAngle is the cosine of the angle between the radiatorThickness\n"
		"normal and the trajectory vector of the charged particle, and n is the refractive index of the material\n"
		"for the given wavelength. To understand the meaning of 'someDistance' follow the provided reference.\n"
		"1. B.Djurnic, A.Potylitsyn, A.Bogdanov, S.Gogolev, On the rework and development of new Geant4\n"
		"Cherenkov models, JINST 20 (2025) P02008; DOI: 10.1088/1748-0221/20/02/P02008\n\n"
		"NOTE1: this model currently supports only optical photons and does not generate photons in the X - ray\n"
		"region. On the other hand, the base class \"G4BaseChR_Model\" removes all the limitations that exist in\n"
		"the G4Cerenkov class, meaning that the model can consider any kind of refractive index dependencies.\n\n"
		"NOTE2: the aim of this model is to consider a charged particle that crosses a radiator of finite thickness\n"
		"and infinite transverse sizes. The current version of the model can be used only for G4Box and G4Tubs\n"
		"volumes. Optionally, the volumes may be divided into layers. G4Tubs must be filled.\n";
}

#define GetBit(intType, bitNo) (((intType) >> (bitNo)) & 1)
#define SetBitTo1(intType, bitNo) ((intType) | (1 << (bitNo)))
#define SetBitTo0(intType, bitNo) ((intType) & ~(1 << (bitNo)))
static unsigned char o_staticFlag = 0; // I could also go with bit-fields, but this is better, I guess

void G4ThinTargetChR_Model::BuildModelPhysicsTable(const G4ParticleDefinition& aParticleDef) {
	G4BaseChR_Model::BuildModelPhysicsTable(aParticleDef);

	if (G4MTRunManager::GetMasterThreadId() != std::this_thread::get_id())
		return;
	// I believe the following should solve the problem of rebuilding the geometry,
	// but it should be still tested - I haven't
	// The following is not thread safe (I didn't use atomics), but I don't care if
	// I don't capture the flag change... after all those 60s is a random number
	if (GetBit(o_staticFlag, 0))
		return;
	std::thread aThread{ [] {
		// Guess 60s is enough even for slow CPUs
		// Is there a way (like a flag) to know if the geometry has changed between runs?
		std::this_thread::sleep_for(std::chrono::seconds(60));
		o_staticFlag = SetBitTo0(o_staticFlag, 0);
		} };
	aThread.detach();
	o_staticFlag = SetBitTo1(o_staticFlag, 0);
	G4PhysicalVolumeStore* thePhysStore = G4PhysicalVolumeStore::GetInstance();
	G4ExtraOpticalParameters::GetInstance()->ScanAndAddUnregisteredLV();
	if (m_verboseLevel > 0)
		std::cout << "Preparing data for G4ThinTargetChR_Model\n";

	// I need to pass the iterator into the next for loop, so not using range-for
	for (auto thisIterator = thePhysStore->begin(); thisIterator < thePhysStore->end(); thisIterator++) {
		const G4VPhysicalVolume* thisPhysVolume = *thisIterator; // I'd hate it to write that (*iter) all the time
		const G4LogicalVolume* thisLogicVolume = thisPhysVolume->GetLogicalVolume();
		// the unordered_map's operator[] should never create an instance here -
		// ScanAndAddUnregisteredLV was executed previously
		G4CherenkovMatData& matData = G4ExtraOpticalParameters::GetInstance()->FindOrCreateChRMatData(thisLogicVolume);
		if (matData.m_minAxis != 255) // already checked and finished the job - no need to do it again
			continue;
		if (thisLogicVolume->GetNoDaughters() > 0) {
			matData.m_minAxis = 254;
			if (m_verboseLevel > 0) {
				if (!GetBit(o_staticFlag, 1)) {
					PrintJustWarningExplanations(0, thisLogicVolume);
					o_staticFlag = SetBitTo1(o_staticFlag, 1);
				}
				PrintSuccessInfo(thisLogicVolume, matData);
			}
			continue;
		}
		std::map<size_t, const G4VPhysicalVolume*> replicas{};
		replicas.insert({ thisPhysVolume->GetCopyNo(), thisPhysVolume });
		size_t counter = 1;
		// first find if there are any other physical volumes that might be replicas
		for (auto nextIterator = thisIterator + 1; nextIterator < thePhysStore->end(); nextIterator++) {
			const G4VPhysicalVolume* nextPhysVolume = *nextIterator;
			if (thisPhysVolume->GetName() != nextPhysVolume->GetName())
				continue;
			if (thisLogicVolume != nextPhysVolume->GetLogicalVolume())
				continue;
			replicas.insert({ nextPhysVolume->GetCopyNo(), nextPhysVolume });
			counter++;
		}
		if (counter != replicas.size()) {
			matData.m_minAxis = 254;
			if (m_verboseLevel > 0) {
				// in case such conditions are possible (not sure if Geant4 allows it)
				if (!GetBit(o_staticFlag, 2)) {
					PrintJustWarningExplanations(1, thisLogicVolume);
					o_staticFlag = SetBitTo1(o_staticFlag, 2);
				}
				PrintSuccessInfo(thisLogicVolume, matData);
			}
			continue;
		}
		const G4VSolid* aSolid = thisLogicVolume->GetSolid();
		G4AffineTransform aTransform;
		try {
			aTransform = GetLocalToGlobalTransformOfPhysicalVolume(thisPhysVolume);
		}
		catch (const no_mother_physical_volume& err) {
			G4Exception("G4ThinTargetChR_Model::BuildModelPhysicsTable", "FE_ThinChRXX", FatalException, err.what());
		}
		const G4AffineTransform inverseTransform{ aTransform.Inverse() };
		// There are virtual methods so typeid should work
		if (typeid(*aSolid) == typeid(G4Box)) {
			const G4Box* aBoxSolid = dynamic_cast<const G4Box*>(aSolid);
			if (counter == 1) {
				SetBoxPhysicsTableParameters(matData, { aBoxSolid->GetXHalfLength(), aBoxSolid->GetYHalfLength(), aBoxSolid->GetZHalfLength() });
				if (m_verboseLevel > 1)
					PrintSuccessInfo(thisLogicVolume, matData);
				continue;
			} // end of counter == 1
			std::vector<G4ThreeVector> dimensionBuffer;
			// The Vec3 is a point
			dimensionBuffer.reserve(replicas.size());
			dimensionBuffer.emplace_back(); // this is a reference coordinate system -> {0., 0., 0.}
			// as this is box, it's good enough to only test one normal and understand relative rotations
			G4ThreeVector thisZNormal{ aBoxSolid->SurfaceNormal({ 0., 0., aBoxSolid->GetZHalfLength() }) };
			G4bool zeroDegreeBetweenNormals = true;
			for (auto& [key, value] : replicas) {
				if (value == thisPhysVolume)
					continue;
				G4Box* anotherSolid = dynamic_cast<G4Box*>(value->GetLogicalVolume()->GetSolid());
				G4ThreeVector anotherNormal{ anotherSolid->SurfaceNormal({ 0., 0., anotherSolid->GetZHalfLength() }) };
				G4ThreeVector anotherPoint{};
				G4AffineTransform anotherAffine{};
				try {
					anotherAffine = GetLocalToGlobalTransformOfPhysicalVolume(value);
				}
				catch (const no_mother_physical_volume& err) {
					G4Exception("G4ThinTargetChR_Model::BuildModelPhysicsTable", "FE_ThinChRXX", FatalException, err.what());
				}
				anotherAffine *= inverseTransform;
				anotherAffine.ApplyAxisTransform(anotherNormal);
				anotherAffine.ApplyPointTransform(anotherPoint);
				G4double dotProduct = anotherNormal.dot(thisZNormal) - 1.;
				if (dotProduct > 1.E-13 || dotProduct < -1.E-13)
					zeroDegreeBetweenNormals = false;
				dimensionBuffer.emplace_back(anotherPoint.getX(), anotherPoint.getY(), anotherPoint.getZ());
			}
			//I'm not sure if there's a Geant4 class that's like G4ThreeVectors but for integers, thus going with std::array
			std::array<size_t, 3> dimensionNoOfLayers{ 0, 0, 0 };
			//See below at function definition what the meanings of the retuned type (flags) are
			unsigned char returnFlags;
			try {
				returnFlags = HandleDimensionBufferVector(dimensionBuffer.begin(), dimensionBuffer.end(), aBoxSolid, 0, dimensionNoOfLayers);
			}
			catch (unsigned char errValue) {
				returnFlags = errValue;
			}
			/*
			### THE G4BOX APPROACH
			Now, there are a few ways to consider what a volume is and when it should be used.
			For instance, I could consider all copyNo values, and build N volumes, where N <= copyNo.
			On the other hand, to do that, I'll need to waste some additional memory while it's very
			possible that G4ThinTargetChR_Model won't be used (mostly people need only the standard
			model and this one without layers). Therefore, I've decided to do the following:
			1. If any of the 6 through 8 returnFlags is false - volumes can't be used
			2. zeroDegreeBetweenNormals can be false only if returnFlags 0 through 2 are false
			3. If any of the returnFlags sets {0, 3}, {1, 4}, or {2, 5}, has both values true the
			   model can't be used
			4. If returnFlag values 3 through 5 are true, while others are not, the model may be used
			5. If 0 through 2 are true, and 3 through 5 are false, volume will be considered only if
			   there's a specific minimal side (in general model assumes finite thickness and infinite
			   transverse sizes)
			*/
			if (returnFlags == 0xff) {
				matData.m_minAxis = 254;
				if (m_verboseLevel > 0) {
					if (m_verboseLevel > 1)
						PrintJustWarningExplanations(2, thisLogicVolume);
					PrintSuccessInfo(thisLogicVolume, matData);
				}
				continue;
			}
			if (GetBit(returnFlags, 6)) {
				matData.m_minAxis = 254;
				if (m_verboseLevel > 0) {
					if (m_verboseLevel > 1)
						PrintJustWarningExplanations(3, thisLogicVolume);
					PrintSuccessInfo(thisLogicVolume, matData);
				}
				continue;
			}
			if ((GetBit(returnFlags, 0) && GetBit(returnFlags, 3)) ||
				(GetBit(returnFlags, 1) && GetBit(returnFlags, 4)) ||
				(GetBit(returnFlags, 2) && GetBit(returnFlags, 5))) {
				matData.m_minAxis = 254;
				if (m_verboseLevel > 0) {
					if (m_verboseLevel > 1)
						PrintJustWarningExplanations(4, thisLogicVolume);
					PrintSuccessInfo(thisLogicVolume, matData);
				}
				continue;
			}
			if (!zeroDegreeBetweenNormals) {
				if (!GetBit(returnFlags, 0) && !GetBit(returnFlags, 1) && !GetBit(returnFlags, 2)) {
					//I haven't tested overlapping here, but it is assumed that volumes don't overlap
					SetBoxPhysicsTableParameters(matData, { aBoxSolid->GetXHalfLength(), aBoxSolid->GetYHalfLength(), aBoxSolid->GetZHalfLength() });
					if (m_verboseLevel > 0)
						PrintSuccessInfo(thisLogicVolume, matData, &dimensionNoOfLayers);
					continue;
				}
				else {
					matData.m_minAxis = 254;
					if (m_verboseLevel > 0) {
						if (m_verboseLevel > 1)
							PrintJustWarningExplanations(5, thisLogicVolume);
						PrintSuccessInfo(thisLogicVolume, matData);
					}
					continue;
				}
			}
			if (returnFlags < 0x40 && !GetBit(returnFlags, 0) && !GetBit(returnFlags, 1) && !GetBit(returnFlags, 2)) {
				
				SetBoxPhysicsTableParameters(matData, { aBoxSolid->GetXHalfLength(), aBoxSolid->GetYHalfLength(), aBoxSolid->GetZHalfLength() });
				if (m_verboseLevel > 0)
					PrintSuccessInfo(thisLogicVolume, matData);
				continue;
			}
			else if (returnFlags < 0x08) {
				//after sorting the dimensionBuffer, the first point is the most (-x, -y, -z) or (0., 0., 0.) point
				//thus, relative to that one, I can calculate where the middle of the layer is. Still, before that,
				//I need to determine the dominant axis
				G4ThreeVector allHalfThickness{ (G4double)dimensionNoOfLayers[0] * aBoxSolid->GetXHalfLength(),
					(G4double)dimensionNoOfLayers[1] * aBoxSolid->GetYHalfLength(), (G4double)dimensionNoOfLayers[2] * aBoxSolid->GetZHalfLength() };
				SetBoxPhysicsTableParameters(matData, allHalfThickness);
				if (matData.m_minAxis == 254)
					continue;
				//and now I finally need to find the point to the middle of the volume
				delete matData.p_middlePoint; // just in case of some bug, but this should always be a nullptr
				matData.p_middlePoint = new G4ThreeVector{
					dimensionBuffer.front().getX() + allHalfThickness.getX() - aBoxSolid->GetXHalfLength(),
					dimensionBuffer.front().getY() + allHalfThickness.getY() - aBoxSolid->GetYHalfLength(),
					dimensionBuffer.front().getZ() + allHalfThickness.getZ() - aBoxSolid->GetZHalfLength()};
				aTransform.ApplyPointTransform(*matData.p_middlePoint); //this is the critical point if using active transformations
				if (m_verboseLevel > 0)
					PrintSuccessInfo(thisLogicVolume, matData, &dimensionNoOfLayers);
				continue;
			}
			else {
				if (m_verboseLevel > 0)
					PrintJustWarningExplanations(6, thisLogicVolume);
				matData.m_minAxis = 254;
				continue;
			}
		} // end of G4Box
		else if (typeid(*aSolid) == typeid(G4Tubs)) {
			const G4Tubs* aTubsSolid = dynamic_cast<const G4Tubs*>(aSolid);
			G4double halfThickness = aTubsSolid->GetZHalfLength();
			if (aTubsSolid->GetInnerRadius() != 0. ||
				aTubsSolid->GetStartPhiAngle() != 0. ||
				aTubsSolid->GetDeltaPhiAngle() != CLHEP::twopi) {
				// in general, I could have checked if other layers can form a filled G4Tubs, but for
				// now I don't care - this is simpler
				matData.m_minAxis = 254;
				if (m_verboseLevel > 0) {
					if (!GetBit(o_staticFlag, 3)) {
						PrintJustWarningExplanations(7, thisLogicVolume, aTubsSolid);
						o_staticFlag = SetBitTo1(o_staticFlag, 3);
					}
					PrintSuccessInfo(thisLogicVolume, matData);
				}
				continue;
			} // end of G4Tubs not filled
			// not checking outer radius for throwing warnings... I assume the one who uses this
			// model knows what it does and its limits (see the process description).
			// After all, the model must be accessed manually for a specific volume
			if (counter == 1) {
				matData.m_halfThickness = halfThickness;
				matData.m_minAxis = 2;
				if (m_verboseLevel > 0)
					PrintSuccessInfo(thisLogicVolume, matData);
				continue;
			}
			using thePairType = std::pair<G4ThreeVector, G4ThreeVector>;
			std::vector<thePairType> zBuffer;
			// Similar principle to the rasterization zBuffer
			// The 1st Vec3 is a point, and the 2nd is a normal
			zBuffer.reserve(replicas.size());
			G4ThreeVector thisNormal{ aTubsSolid->SurfaceNormal({ 0., 0., halfThickness }) };
			zBuffer.emplace_back(G4ThreeVector{}, thisNormal);
			for (auto& [theFirst, theSecond] : replicas) {
				if (theSecond == thisPhysVolume)
					continue;
				G4Tubs* anotherSolid = dynamic_cast<G4Tubs*>(theSecond->GetLogicalVolume()->GetSolid());
				G4ThreeVector anotherNormal{ anotherSolid->SurfaceNormal({ 0., 0., anotherSolid->GetZHalfLength() }) };
				G4ThreeVector anotherPoint{};
				G4AffineTransform anotherAffine{};
				try {
					anotherAffine = GetLocalToGlobalTransformOfPhysicalVolume(theSecond);
				}
				catch (const no_mother_physical_volume& err) {
					G4Exception("G4ThinTargetChR_Model::BuildModelPhysicsTable", "FE_ThinChRXX", FatalException, err.what());
				}
				anotherAffine *= inverseTransform;
				anotherAffine.ApplyAxisTransform(anotherNormal);
				anotherAffine.ApplyPointTransform(anotherPoint);

				zBuffer.emplace_back(std::move(anotherPoint), std::move(anotherNormal));
			}
			std::sort(zBuffer.begin(), zBuffer.end(), [](const thePairType& value1, const thePairType& value2)
				{ return value1.first.getZ() < value2.first.getZ(); });
			/*
			### THE G4TUBS APPROACH
			Now, there are a few ways to consider what a volume is and when it should be used.
			For instance, I could consider all copyNo values, and build N volumes, where N <= copyNo.
			On the other hand, to do that, I'll need to waste some additional memory while it's very
			possible that G4ThinTargetChR_Model won't be used (mostly people need only the standard
			model and this one without layers). Therefore, I've decided to do the following:
			1. Check if all layers are glued together. If they are, consider them as one
			2. Also, besides checking the coordinate, make sure that the dot product is 1
			3. If they are not glued together (not a single one), consider all layers as a new radiator
			4. If some are glued together, but not all of them, passing m_aSolidType = G4ChRSolidType::notUsableShape
			   and the model may not produce photons in such a volume (one should use the standard model)
			*/
			const G4double theLayerThickness = 2 * halfThickness;
			G4bool breakInLayers = false;
			G4bool foundGluedLayers = false;
			G4bool withOffset = false;
			G4double dotProduct, r;
			for (size_t i = 0; i < zBuffer.size() - 1; i++) {
				// also, could have used the tolerance instead of 1.E-13
				if (zBuffer[i + 1].first.getZ() - zBuffer[i].first.getZ() - theLayerThickness > 1.E-13) //IEEE754
					breakInLayers = true; //could have also tested < -1.E-13 for overlapping, but I guess it's not necessary
				else
					foundGluedLayers = true;
				// no need for sqrt of r -> r should be == 0
				r = zBuffer[i].first.getX() * zBuffer[i].first.getX() + zBuffer[i].first.getY() * zBuffer[i].first.getY();
				if (r > 1.E-13)
					withOffset = true;
				dotProduct = zBuffer[i].second.dot(thisNormal) - 1.;
				if (dotProduct > 1.E-13 || dotProduct < -1.E-13)
					breakInLayers = true;
			}
			// now the last layer that wasn't tested
			r = zBuffer.back().first.getX() * zBuffer.back().first.getX() + zBuffer.back().first.getY() * zBuffer.back().first.getY();
			if (r > 1.E-13)
				withOffset = true;
			dotProduct = zBuffer.back().second.dot(thisNormal) - 1.;
			if (dotProduct > 1.E-13 || dotProduct < -1.E-13)
				breakInLayers = true;
			// And finally, now I only need to test all the flags
			if (withOffset && foundGluedLayers) {
				matData.m_minAxis = 254;
				if (m_verboseLevel > 0) {
					if (m_verboseLevel > 1)
						PrintJustWarningExplanations(8, thisLogicVolume);
					PrintSuccessInfo(thisLogicVolume, matData);
				}
				continue;
			}
			if (!breakInLayers && foundGluedLayers) {
				// it's a layered radiator
				matData.m_halfThickness = (G4double)counter * halfThickness;
				delete matData.p_middlePoint; // just in case of some bug, but this should always be a nullptr
				// the minimal z (it's 0. or negative) + the total half thickness - the half thickness as it's missing
				// (it's missing because coordinates of each layers were sampled as (0., 0., 0.))
				matData.p_middlePoint = 
					new G4ThreeVector{ 0., 0., zBuffer.front().first.getZ() + matData.m_halfThickness - halfThickness };
				aTransform.ApplyPointTransform(*matData.p_middlePoint); //this is the critical point if using active transformations
				matData.m_minAxis = 2;
				if (m_verboseLevel > 0) {
					std::array<size_t, 3> noOfLayers{ 0, 0, zBuffer.size() };
					PrintSuccessInfo(thisLogicVolume, matData, &noOfLayers);
				}
				continue;
			}
			else if (breakInLayers && !foundGluedLayers) {
				// all layers are separate and can be considered separate volumes
				matData.m_halfThickness = halfThickness;
				matData.m_minAxis = 2;
				if (m_verboseLevel)
					PrintSuccessInfo(thisLogicVolume, matData);
				continue;
			}
			else {
				// mixed layers (some glued some not); this model should not be used
				matData.m_minAxis = 254;
				if (m_verboseLevel > 0) {
					if (m_verboseLevel > 1)
						PrintJustWarningExplanations(9, thisLogicVolume);
					PrintSuccessInfo(thisLogicVolume, matData);
				}
				continue;
			}
		} // end of G4Tubs
		else {
			matData.m_minAxis = 254;
			if (m_verboseLevel > 0) {
				if (!GetBit(o_staticFlag, 4)) {
					PrintJustWarningExplanations(10, thisLogicVolume);
					o_staticFlag = SetBitTo1(o_staticFlag, 4);
				}
				PrintSuccessInfo(thisLogicVolume, matData);
			}
		}
	}
}

//=========private G4ThinTargetChR_ModelMessenger:: methods=========
G4bool G4ThinTargetChR_Model::FindParticleEntryAndExitPoints(
		G4ThreeVector& localEntryPoint,
		G4ThreeVector& localExitPoint,
		const G4ThreeVector& localMiddlePoint,
		const G4ThreeVector& localDirection,
		const G4ThreeVector& localPrePoint,
		const G4CherenkovMatData& aChRMatData) const {

	// newPoint = oldPoint + intensity * unitVector
	G4double intensity;
	// keeping calculations relative to the localEntryPoint, not to the emission point of photons
	// that should be still considered... it would cause some problems
	if (aChRMatData.m_minAxis == 0) {
		if (localDirection.getX() == 0.) {
			if (m_verboseLevel > 0)
				PrintJustWarningExplanations(11);
			return false;
		}
		intensity = (-aChRMatData.m_halfThickness + localMiddlePoint.getX() - localPrePoint.getX()) / localDirection.getX();
		localEntryPoint.set(-aChRMatData.m_halfThickness + localMiddlePoint.getX(),
			localPrePoint.getY() + intensity * localDirection.getY(),
			localPrePoint.getZ() + intensity * localDirection.getZ());
		intensity = (aChRMatData.m_halfThickness + localMiddlePoint.getX() - localPrePoint.getX()) / localDirection.getX();
		localExitPoint.set(aChRMatData.m_halfThickness + localMiddlePoint.getX(),
			localPrePoint.getY() + intensity * localDirection.getY(),
			localPrePoint.getZ() + intensity * localDirection.getZ());
		if (localDirection.getX() < 0)
			std::swap(localEntryPoint, localExitPoint);
	} // end of "aChRMatData.m_minAxis == 0"
	else if (aChRMatData.m_minAxis == 1) {
		if (localDirection.getY() == 0.) {
			if (m_verboseLevel > 0)
				PrintJustWarningExplanations(12);
			return false;
		}
		intensity = (-aChRMatData.m_halfThickness + localMiddlePoint.getY() - localPrePoint.getY()) / localDirection.getY();
		localEntryPoint.set(localPrePoint.getX() + intensity * localDirection.getX(),
			-aChRMatData.m_halfThickness + localMiddlePoint.getY(),
			localPrePoint.getZ() + intensity * localDirection.getZ());
		intensity = (aChRMatData.m_halfThickness + localMiddlePoint.getY() - localPrePoint.getY()) / localDirection.getY();
		localExitPoint.set(localPrePoint.getX() + intensity * localDirection.getX(),
			aChRMatData.m_halfThickness + localMiddlePoint.getY(),
			localPrePoint.getZ() + intensity * localDirection.getZ());
		if (localDirection.getY() < 0)
			std::swap(localEntryPoint, localExitPoint);
	} // end of "aChRMatData.m_minAxis == 1"
	else /*aChRMatData.m_minAxis == 2*/ {
		if (localDirection.getZ() == 0.) {
			if (m_verboseLevel > 0)
				PrintJustWarningExplanations(13);
			return false;
		}
		intensity = (-aChRMatData.m_halfThickness + localMiddlePoint.getZ() - localPrePoint.getZ()) / localDirection.getZ();
		localEntryPoint.set(localPrePoint.getX() + intensity * localDirection.getX(),
			localPrePoint.getY() + intensity * localDirection.getY(),
			-aChRMatData.m_halfThickness + localMiddlePoint.getZ());
		intensity = (aChRMatData.m_halfThickness + localMiddlePoint.getZ() - localPrePoint.getZ()) / localDirection.getZ();
		localExitPoint.set(localPrePoint.getX() + intensity * localDirection.getX(),
			localPrePoint.getY() + intensity * localDirection.getY(),
			aChRMatData.m_halfThickness + localMiddlePoint.getZ());
		if (localDirection.getZ() < 0)
			std::swap(localEntryPoint, localExitPoint);
	} // end of "aChRMatData.m_minAxis == 2"
	return true;
}

G4double G4ThinTargetChR_Model::CalculateGaussSigmaDistance(
		const G4ThreeVector& localEntryPoint,
		const G4ThreeVector& localExitPoint,
		const G4ThreeVector& photonDirection,
		const G4CherenkovMatData& matData) const {
	
	G4double neededResult;
	if (matData.m_minAxis == 0) {
		if (photonDirection.getX() == 0.)
			return DBL_MAX;
		// 2 * matData.m_halfThickness == localExitPoint.getX() - localEntryPoint.getX()
		G4double intensity = 2 * matData.m_halfThickness / photonDirection.getX();
		G4ThreeVector exitPhotonPoint =
			G4ThreeVector{
				localExitPoint.getX(),
				localEntryPoint.getY() + intensity * photonDirection.getY(),
				localEntryPoint.getZ() + intensity * photonDirection.getZ() };
		neededResult = (localExitPoint - exitPhotonPoint).mag();
	}
	else if (matData.m_minAxis == 1) {
		if (photonDirection.getY() == 0.)
			return DBL_MAX;
		// 2 * matData.m_halfThickness == localExitPoint.getY() - localEntryPoint.getY()
		G4double intensity = 2 * matData.m_halfThickness / photonDirection.getY();
		G4ThreeVector exitPhotonPoint =
			G4ThreeVector{
				localEntryPoint.getX() + intensity * photonDirection.getX(),
				localExitPoint.getY(),
				localEntryPoint.getZ() + intensity * photonDirection.getZ() };
		neededResult = (localExitPoint - exitPhotonPoint).mag();
	}
	else /*for tubs and box z-coordinate it's the same code*/ {
		if (photonDirection.getZ() == 0.)
			return DBL_MAX;
		// 2 * matData.m_halfThickness == localExitPoint.getZ() - localEntryPoint.getZ()
		G4double intensity = 2 * matData.m_halfThickness / photonDirection.getZ();
		G4ThreeVector exitPhotonPoint =
			G4ThreeVector{
				localEntryPoint.getX() + intensity * photonDirection.getX(),
				localEntryPoint.getY() + intensity * photonDirection.getY(),
				localExitPoint.getZ() };
		neededResult = (localExitPoint - exitPhotonPoint).mag();
	}
	/*
	While it might be better to make sure that the "exitPhotonPoint < transverseSize"
	condition is satisfied, I'm not checking that here, i.e., if the user is using
	the class for thin targets with big transverse sizes (as intended), there should
	be no problems.
	Or is it maybe better to introduce safeties, at least for verboseLevel > 0?
	*/
	return neededResult;
}

void G4ThinTargetChR_Model::SetBoxPhysicsTableParameters(G4CherenkovMatData& matData, const G4ThreeVector& halfThickness) const {
	G4bool singleMinimal = true;
	G4double minSize = halfThickness.getX();
	matData.m_minAxis = 0;
	if (halfThickness.getY() < minSize) {
		minSize = halfThickness.getY();
		matData.m_minAxis = 1;
	}
	else if (halfThickness.getY() == minSize)
		singleMinimal = false;
	if (halfThickness.getZ() < minSize) {
		singleMinimal = true;
		minSize = halfThickness.getZ();
		matData.m_minAxis = 2;
	}
	else if (halfThickness.getZ() == minSize)
		singleMinimal = false;
	if (!singleMinimal) {
		if (m_verboseLevel > 0 && !GetBit(o_staticFlag, 5)) {
			PrintJustWarningExplanations(14);
			o_staticFlag = SetBitTo1(o_staticFlag, 5);
		}
		matData.m_minAxis = 254;
		return;
	}
	matData.m_halfThickness = minSize;
}

#define Combine1sOfChars(lhvIntType, rhvIntType) ((lhvIntType) | (rhvIntType))
//=========static methods of translation unit=========
/*
The following function return bit flags:
0 - found layers that are glued together in z-direction
1 - found layers that are glued together in y-direction
2 - found layers that are glued together in x-direction
3 - found layers that are not glued together in z-direction
4 - found layers that are not glued together in y-direction
5 - found layers that are not glued together in x-direction
6 - not all layers have the same number of volumes - fatal error and not using the model
0xff - fatal unexpected error (e.g., overlap of volumes)
*/
[[nodiscard]] static unsigned char HandleDimensionBufferVector(const dimBufferIter_t begin, const dimBufferIter_t end, const G4Box* aBoxSolid,
		const unsigned char passNo, std::array<size_t, 3>& dimensionNoOfLayers) {
	// I've tested this function, but it should be test with many more volumes
	
	unsigned char dimension = (unsigned char)(2 - passNo);
	std::sort(begin, end, [dimension](const G4ThreeVector& lhv, const G4ThreeVector& rhv)
		{return lhv[dimension] < rhv[dimension]; });
	unsigned char theReturnValue = 0;
	dimBufferIter_t nextBegin = begin;
	dimBufferIter_t previousBegin = begin;
	const unsigned char nextPassNo = (unsigned char)(passNo + 1);
	G4double halfThickness;
	size_t theCount = (size_t)(end - begin);
	if (passNo == 0)
		halfThickness = aBoxSolid->GetZHalfLength();
	else if (passNo == 1)
		halfThickness = aBoxSolid->GetYHalfLength();
	else
		halfThickness = aBoxSolid->GetXHalfLength();
	while (true) {
		dimBufferIter_t nextEnd;
		size_t thisLayer;
		if (dimension != 0) {
			nextEnd = std::lower_bound(nextBegin, end, (*nextBegin)[dimension] + 2 * halfThickness,
				[dimension](const G4ThreeVector& lhv, const G4double rhv) { return lhv[dimension] - rhv < -1.E-13; });
			theReturnValue = Combine1sOfChars(theReturnValue, HandleDimensionBufferVector(nextBegin, nextEnd, aBoxSolid, nextPassNo, dimensionNoOfLayers));
			thisLayer = (size_t)(nextEnd - nextBegin);
		}
		else {
			thisLayer = 1;
		}
		size_t dimensionLength = 0;
		size_t remainder = 0;
		if (thisLayer != 0) {
			dimensionLength = theCount / thisLayer;
			remainder = theCount % thisLayer;
		}
		// the first one goes to the following if and it's at least 1, i.e., no way dimensionLength == 0
		if (dimensionNoOfLayers[dimension] == 0) {
			dimensionNoOfLayers[dimension] = dimensionLength;
			if (dimension == 0) {
				for (; nextBegin + 1 < end; nextBegin++) {
					G4double spacing = (*(nextBegin + 1))[dimension] - (*nextBegin)[dimension] - 2 * halfThickness;
					if (spacing < -1.E-13) // overlapping volumes
						throw (unsigned char)0xff;
					else if (spacing > 1.E-13) // some distance between layers
						theReturnValue = Combine1sOfChars(theReturnValue, SetBitTo1(0, 3 + passNo));
					else // glued layers
						theReturnValue = Combine1sOfChars(theReturnValue, SetBitTo1(0, passNo));
				}
				return theReturnValue;
			}
		}
		else {
			size_t previousLayer = (size_t)(nextBegin - previousBegin);
			if (remainder != 0)
				throw (unsigned char)Combine1sOfChars(theReturnValue, SetBitTo1(0, 6));
			if (dimension == 0) {
				for (; nextBegin + 1 < end; nextBegin++) {
					G4double spacing = (*(nextBegin + 1))[dimension] - (*nextBegin)[dimension] - 2 * halfThickness;
					if (spacing < -1.E-13) // overlapping volumes
						throw (unsigned char)0xff;
					else if (spacing > 1.E-13) // some distance between layers
						theReturnValue = Combine1sOfChars(theReturnValue, SetBitTo1(0, 3 + passNo));
					else // glued layers
						theReturnValue = Combine1sOfChars(theReturnValue, SetBitTo1(0, passNo));
				}
				return theReturnValue;
			}
			else if (previousLayer == 0){} // for y and z directions, the first one should not be tested
			else if (previousLayer == thisLayer) { // always entering here for 'dimension == 0'
				for (size_t i = 0; i < previousLayer; i++) {
					G4double spacing = (*(nextBegin + i))[dimension] - (*(previousBegin + i))[dimension] - 2 * halfThickness;
					if (spacing < -1.E-13) // overlapping volumes
						throw (unsigned char)0xff;
					else if (spacing > 1.E-13) // some distance between layers
						theReturnValue = Combine1sOfChars(theReturnValue, SetBitTo1(0, 3 + passNo));
					else // glued layers
						theReturnValue = Combine1sOfChars(theReturnValue, SetBitTo1(0, passNo));
				}
			}
			else // I think this should never happen bcs of 'if (remainder != 0)'
				throw (unsigned char)0xff;
		}
		previousBegin = nextBegin;
		nextBegin = nextEnd;
		if (nextEnd == end)
			return theReturnValue;
	}
}

static void PrintJustWarningExplanations(const size_t id, const void* aType1, const void* aType2) {
	if (id == 0) {
		auto* aLogic = static_cast<const G4LogicalVolume*>(aType1);
		std::ostringstream err;
		err << "A logical volume " << std::quoted(aLogic->GetName()) <<
			"\nhas daughters which means it cannot produce Cherenkov\n"
			"photons in G4ThinTargetChR_Model. Only volumes without\n"
			"registered daughters may use this model of Cherenkov radiation.\n"
			"Still, daughters of this volume might be able to use this model!\n"
			"The same applies for other mother volumes, if there are any.\n";
		G4Exception("G4ThinTargetChR_Model::BuildModelPhysicsTable", "WE_ThinChR00", JustWarning, err);
	}
	else if (id == 1) {
		auto* aLogic = static_cast<const G4LogicalVolume*>(aType1);
		std::ostringstream err;
		err << "Found physical volumes with the same copy number,\n"
			"name and logical volume. If you want to use this class,\n"
			"please make sure that parametrized volumes have the same\n"
			"names of physical volumes, the same pointers of logical\n"
			"volumes, and different copy numbers.\n"
			"This class will not be executed in this run for:\n"
			<< std::quoted(aLogic->GetName()) << "!\n"
			"The same applies for all other such volumes.\n";
		G4Exception("G4ThinTargetChR_Model::BuildModelPhysicsTable", "WE_ThinChR01", JustWarning, err);
	}
	else if (id == 2) {
		auto* aLogic = static_cast<const G4LogicalVolume*>(aType1);
		std::ostringstream err;
		err << "Volume " << std::quoted(aLogic->GetName()) << " can't be used\n"
			"in G4ThinTargetChR_Model. Some unexpected error has occurred,\n"
			"e.g., some of the layers overlap. If that's not the case,\n"
			"please, report this error\n";
		G4Exception("G4ThinTargetChR_Model::BuildModelPhysicsTable", "WE_ThinChR02", JustWarning, err);
	}
	else if (id == 3) {
		auto* aLogic = static_cast<const G4LogicalVolume*>(aType1);
		std::ostringstream err;
		err << "Volume " << std::quoted(aLogic->GetName()) << " can't be used\n"
			"in G4ThinTargetChR_Model. It is registered that the number of layers\n"
			"in some directions changes. This model currently doesn't support that.\n";
		G4Exception("G4ThinTargetChR_Model::BuildModelPhysicsTable", "WE_ThinChR03", JustWarning, err);
	}
	else if (id == 4) {
		auto* aLogic = static_cast<const G4LogicalVolume*>(aType1);
		std::ostringstream err;
		err << "Volume " << std::quoted(aLogic->GetName()) << " can't be used\n"
			"in G4ThinTargetChR_Model. To use this model for G4Box volumes, make\n"
			"sure that volumes are either all glued or all separated.\n";
		G4Exception("G4ThinTargetChR_Model::BuildModelPhysicsTable", "WE_ThinChR04", JustWarning, err);
	}
	else if (id == 5) {
		auto* aLogic = static_cast<const G4LogicalVolume*>(aType1);
		std::ostringstream err;
		err << "Volume " << std::quoted(aLogic->GetName()) << " can't be used\n"
			"in G4ThinTargetChR_Model. Some of the layers are rotated relative\n"
			"to each other while some of the are glued as well. To use this\n"
			"model, either make sure they layers are not rotated one to another\n"
			"or make sure that no a single one of them is glued.";
		G4Exception("G4ThinTargetChR_Model::BuildModelPhysicsTable", "WE_ThinChR05", JustWarning, err);
	}
	else if (id == 6) {
		auto* aLogic = static_cast<const G4LogicalVolume*>(aType1);
		std::ostringstream err;
		err << "Volume " << std::quoted(aLogic->GetName()) << " can't be used\n"
			"in G4ThinTargetChR_Model. This is a general message which means you\n"
			"used some non-standard and unsupported distribution of G4Box layers.\n";
		G4Exception("G4ThinTargetChR_Model::BuildModelPhysicsTable", "WE_ThinChR06", JustWarning, err);
	}
	else if (id == 7) {
		auto* aLogic = static_cast<const G4LogicalVolume*>(aType1);
		auto* aTubsSolid = static_cast<const G4Tubs*>(aType2);
		std::ostringstream err;
		err << "Currently, G4ThinTargetChR_Model supports only\n"
			"completely filled G4Tubs volumes, i.e., if you make\n"
			"holes in the solid, you won't be able to use the model\n"
			"Volume " << std::quoted(aLogic->GetName())
			<< ", however has:\nInner radius: " << aTubsSolid->GetInnerRadius() / mm
			<< " mm\nStart phi angle: " << aTubsSolid->GetStartPhiAngle() / deg
			<< "deg\nDelta phi angle: " << aTubsSolid->GetDeltaPhiAngle() / deg
			<< "deg\nThe model for this volume may not be used in simulations!\n";
		G4Exception("G4ThinTargetChR_Model::BuildModelPhysicsTable", "WE_ThinChR07", JustWarning, err);
	}
	else if (id == 8) {
		auto* aLogic = static_cast<const G4LogicalVolume*>(aType1);
		std::ostringstream err;
		err << "Volume " << std::quoted(aLogic->GetName()) << " is detected\n"
			"as a layered volume and may not be used in the G4ThinTargetChR_Model\n"
			"because some layers have an offset while some are glued together.\n";
		G4Exception("G4ThinTargetChR_Model::BuildModelPhysicsTable", "WE_ThinChR08", JustWarning, err);
	}
	else if (id == 9) {
		auto* aLogic = static_cast<const G4LogicalVolume*>(aType1);
		std::ostringstream err;
		err << "Volume " << std::quoted(aLogic->GetName()) << " is detected\n"
			"as a layered volume and may not be used in the G4ThinTargetChR_Model\n"
			"because some layers are placed together, while some are kept separate.\n";
		G4Exception("G4ThinTargetChR_Model::BuildModelPhysicsTable", "WE_ThinChR09", JustWarning, err);
	}
	else if (id == 10) {
		const char* err = "Currently, G4ThinTargetChR_Model can be used only for G4Tubs and G4Box\n"
			"volumes and only the thickness (a single dimension) should be\n"
			"small (e.g., '1 mm' and less). If you want to produce Cherenkov\n"
			"radiation in thicker targets, use standard models for better\n"
			"for better performance (results won't differ).\n"
			"If, however, you need more complex shapes, currently, this model\n"
			"cannot consider them correctly.\n"
			"Note that only fully filled G4Tubs is supported!\n";
		G4Exception("G4ThinTargetChR_Model::BuildModelPhysicsTable", "WE_ThinChR10", JustWarning, err);
	}
	else if (id == 11) {
		const char* err = "A particle moving parallel to the G4Box x-surfaces!\n"
			"Obtaining this message in your simulations might mean\n"
			"that you should not use G4ThinTargetChR_Model in your\n"
			"simulations. This model should be used when a charged\n"
			"moves along the thickness dimension of a thin target,\n"
			"while other two dimensions are consider infinite!\n"
			"Cherenkov photons not emitted!\n";
		G4Exception("G4ThinTargetChR_Model::FindParticleEntryAndExitPoints", "WE_ThinChR11", JustWarning, err);
	}
	else if (id == 12) {
		const char* err = "A particle moving parallel to the G4Tubs y-surfaces!\n"
			"Obtaining this message in your simulations might mean\n"
			"that you should not use G4ThinTargetChR_Model in your\n"
			"simulations. This model should be used when a charged\n"
			"moves along the thickness dimension of a thin target,\n"
			"while other two dimensions are consider infinite!\n"
			"Cherenkov photons not emitted!\n";
		G4Exception("G4ThinTargetChR_Model::FindParticleEntryAndExitPoints", "WE_ThinChR12", JustWarning, err);
	}
	else if (id == 13) {
		const char* err = "A particle moving parallel to the G4Tubs z-surfaces!\n"
			"Obtaining this message in your simulations might mean\n"
			"that you should not use G4ThinTargetChR_Model in your\n"
			"simulations. This model should be used when a charged\n"
			"moves along the thickness dimension of a thin target,\n"
			"while other two dimensions are consider infinite!\n"
			"Cherenkov photons not emitted!\n";
		G4Exception("G4ThinTargetChR_Model::FindParticleEntryAndExitPoints", "WE_ThinChR13", JustWarning, err);
	}
	else if (id == 14) {
		const char* err = "To use G4ThinTargetChR_Model for G4Box volumes (or layered ones),\n"
			"make sure that there is a dominant dimension, i.e., its size is lower than of\n"
			"the other two. The model assumes only one dimension is of a finite thickness,\n"
			"while the transverse sizes are assumed to be infinite.";
		G4Exception("G4ThinTargetChR_Model::SetBoxPhysicsTableParameters", "WE_ThinChR14", JustWarning, err);
	}
	else {
		const char* err = "Invalid provided id!\n";
		G4Exception("static PrintJustWarningExplanations; file: G4ThinTargetChR_Model.cc", "FT_ThinChRXX", FatalException, err);
	}
}

static void PrintSuccessInfo(const G4LogicalVolume* aLogic, const G4CherenkovMatData& matData, std::array<size_t, 3>* noLayersPerDimension) {
	std::cout << "---------------------------------------------------\n";
	std::cout << "Loaded data used in G4ThinTargetChR_Model for " << std::quoted(aLogic->GetName()) << ":\n"
		<< std::setw(47) << std::setfill(' ') << ' ' << std::setfill('^') << std::setw(aLogic->GetName().length())
		<< "^" << '\n';
	if (matData.GetMinAxis() > 2) {
		std::cout << "This logical volume may not be used in G4ThinTargetChR_Model.\n"
			"Consider using a standard Cherenkov model (G4StandardChR_Model)\n"
			"---------------------------------------------------\n";
		return;
	}
	std::cout << std::right << std::setfill(' ');
	if (noLayersPerDimension) {
		std::cout << "The volume is detected as a layered volume!\n";
		if ((*noLayersPerDimension)[0] == 0 && (*noLayersPerDimension)[1] == 0) // this is for G4Tubs
			std::cout << "The detected number of z G4Tubs layers is:\n" << std::setw(40) << ' ' << (*noLayersPerDimension)[2] << '\n';
		else {
			std::cout << "The detected number of G4Box layers is:\n"
				<< std::setw(40)
				<< "in local x direction: " << (*noLayersPerDimension)[0] << '\n'
				<< std::setw(40)
				<< "in local y direction: " << (*noLayersPerDimension)[1] << '\n'
				<< std::setw(40)
				<< "in local z direction: " << (*noLayersPerDimension)[2] << '\n';
		}
		std::cout << "Global coordinates of the center of the layer are:\n"
			<< std::setw(10) << matData.GetMiddlePointVec() << '\n';
		std::cout << std::left << std::setw(40);
		std::cout << "Volume minimal thickness direction: ";
		if (matData.GetMinAxis() == 0)
			std::cout << "x\n";
		else if (matData.GetMinAxis() == 1)
			std::cout << "y\n";
		else
			std::cout << "z\n";
	}
	else
		std::cout << "The volume is not a layered volume!\n"; 
	std::cout << std::left << std::setw(40) << "Volume half thickness: " << matData.GetHalfThickness() / um << " um\n"
		"---------------------------------------------------\n";
	std::cout << std::right; // just to return it to default
}