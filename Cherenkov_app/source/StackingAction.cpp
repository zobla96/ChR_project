//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "StackingAction.hpp"
//G4 headers
#include "G4AnalysisManager.hh"

beginChR

//=========public ChR::StackingAction:: methods=========

G4ClassificationOfNewTrack StackingAction::ClassifyNewTrack(const G4Track* aTrack) {
	if (aTrack->GetTrackID() <= (int)PrimaryGeneratorAction::GetInstance()->GetNoOfParticles())
		return fUrgent;
#ifdef standardRun
	else {
		if (aTrack->GetParticleDefinition()->GetParticleName() == "opticalphoton") {
			if (aTrack->GetMomentumDirection().y() <= 0)
				//Those wouldn't be detected while they would increase the computation time significantly
				//Due to high refractive index, they would experience total internal reflections
				return fKill;
			else
				//The condition might be improved even further for even faster computations
				//(maybe in some subsequent version)
				return fUrgent;
		}
	}
#else
	if (aTrack->GetParticleDefinition()->GetParticleName() == "opticalphoton") {
		/*
		for checking performance, undef "captureChRPhotonEnergyDistribution".
		It significantly slows down a multi-thread app, as if some
		atomics are a problem (haven't analyzed). The previous "as if" is
		because I got better performance on 4 threads compared to 20
		which can be related to atomics' cache memory effects!
		*/
#ifdef captureChRPhotonEnergyDistribution
		G4double energy = aTrack->GetTotalEnergy();
		G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
		analysisManager->FillNtupleIColumn(0, 0);
		analysisManager->FillNtupleDColumn(1, energy / eV);
		analysisManager->FillNtupleDColumn(2, 1.239841984e-6 * m * eV / (energy * nm));
		analysisManager->FillNtupleDColumn(3, 0.);
		analysisManager->FillNtupleDColumn(4, 0.);
		analysisManager->AddNtupleRow();
#endif // captureChRPhotonEnergyDistribution
	}
#endif // standardRun
	//While the following return ain't perfect, additional particles could only raise the background.
	//Due to the geometry, the raise in peaks' intensity would be insignificant (I believe).
	//Yet, those might increase the already long computation times
	return fKill;
}

endChR