//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//G4 headers
#include "G4StandardChR_Model.hh"
#include "G4ParticleChange.hh"
#include "G4Poisson.hh"
#include "G4OpticalPhoton.hh"
#include "G4OpticalParameters.hh"
#include "G4ExtraOpticalParameters.hh"
#include "SomeGlobalNamespace.hh"

//=========public G4StandardChR_Model:: methods=========

G4StandardChR_Model::G4StandardChR_Model(const char* theName)
: G4BaseChR_Model(theName){

}

G4VParticleChange* G4StandardChR_Model::PostStepModelDoIt(const G4Track& aTrack, const G4Step& aStep, const G4CherenkovMatData& aChRMatData) {
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
	if(!RIndex)
		RIndex = aMaterial->GetMaterialPropertiesTable()->GetProperty(kREALRINDEX);

	const G4double charge = aParticle->GetDefinition()->GetPDGCharge();
	const G4double beta = (preStepPoint->GetBeta() + postStepPoint->GetBeta()) * 0.5;

	G4double meanNumberOfPhotons = CalculateAverageNumberOfPhotons(charge, beta, materialID);

	if(meanNumberOfPhotons <= 0.0) {
		//unchanged particle; number of secondaries is 0 anyway after p_particleChange->Initialize(aTrack);
		return p_particleChange;
	}

	meanNumberOfPhotons = meanNumberOfPhotons * aStep.GetStepLength();
	G4int noOfPhotons = (G4int)G4Poisson(meanNumberOfPhotons);

	G4OpticalParameters* optParameters = G4OpticalParameters::Instance();
	//got no idea what's the point of 'optParameters->GetCerenkovStackPhotons()',
	//but staying consistent with G4Cerenkov...
	if(noOfPhotons <= 0 || !optParameters->GetCerenkovStackPhotons())
		return p_particleChange;
	
	p_particleChange->SetNumberOfSecondaries(noOfPhotons);

	if(optParameters->GetCerenkovTrackSecondariesFirst()) {
		if (aTrack.GetTrackStatus() == fAlive)
			p_particleChange->ProposeTrackStatus(fSuspend);
	}

	G4double lossEnergy = 0.;

	// initializing to keep the compiler silent
	G4double minEnergy = 0.;
	G4double maxEnergy = 0.;
	// or would it be better to keep it in stack and check condition *1* every time in the following 'for'??
	std::vector<std::pair<G4double, G4double>>* bigBetaCDFVector = nullptr;
	if (aChRMatData.GetExoticRIndex()) {
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
	
	for(G4int i = 0; i < noOfPhotons; ++i) {
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
			while (true) {
				rand = G4UniformRand();
				sampledEnergy = minEnergy + rand * (maxEnergy - minEnergy);
				sampledRI = RIndex->Value(sampledEnergy);
				cosTheta = 1. / (sampledRI * beta); //might give > 1. for strange n(E) functions
				sin2Theta = (1.0 - cosTheta) * (1.0 + cosTheta);
				if (sin2Theta <= 0.) {
					//the following 'if' is to prevent bad distributions if the user modified an almost non-exotic RIndex
					if (!aChRMatData.GetExoticInitialFlag())
						minEnergy = sampledEnergy;
					continue;
				}
				break;
			}
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

	if (m_useModelWithEnergyLoss) {
		//considering only energy loss, but neglecting change in momentum direction...
		//photons have very low energy and are emitted uniformly around the particle so this should be 
		//really meager, and almost seems like IEEE754 error would be greater than the actual change.
		//Or should I maybe consider momentum direction change as well?
		p_particleChange->ProposeEnergy(p_particleChange->GetEnergy() - lossEnergy);
	}
	if(m_verboseLevel > 1)
		std::cout << "\n Exiting from G4StandardChR_Model::PostStepModelDoIt -- NumberOfSecondaries = "
		<< p_particleChange->GetNumberOfSecondaries() << G4endl;

	if (bigBetaCDFVector && beta > m_ChRPhysDataVec[materialID].m_aroundBetaValues.back().m_betaValue)
		delete bigBetaCDFVector;

	return p_particleChange;
}

void G4StandardChR_Model::DumpModelInfo() const {
	std::cout <<
		"\"G4StandardChR_Model\" is a Cherenkov radiation model that's based on the original Frank-Tamm theory.\n"
		"That means Cherenkov photons are emitted along the lateral surface of the cone relative to a charged\n"
		"particle that passes through the material. The cone angle can be expressed as:\n"
		"cos(ThetaChR) = 1 / (beta * RIndex).\n"
		"To read more about the first Frank-Tamm theory, see:\n"
		"I.M.Frank, I.E.Tamm, Coherent visible radiation of fast electrons passing through matter,\n"
		"Dokl. Acad. Sci. USSR 14 (1937) 109-114\n\n"
		"NOTE1: this model currently supports only optical photons and does not generate photons in the X-ray\n"
		"region. On the other hand, the base class \"G4BaseChR_Model\" removes all the limitations that exist in\n"
		"the G4Cerenkov class, meaning that the model can consider any kind of refractive index dependencies.\n\n"
		"NOTE2: this model should generate good results as long as the considered radiator can be approximated\n"
		"as \"ideal\", i.e., the Frank-Tamm theory is written for an infinitely thick emitter. If thin radiators\n"
		"are considered, one should consider other models. For instance, check \"G4ThinTargetChR_Model\", which\n"
		"allows one to consider thin plate-like radiators (one finite and two infinite dimensions).\n";
}