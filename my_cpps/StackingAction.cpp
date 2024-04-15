#include "StackingAction.hpp"

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

beginChR

//=========public ChR::StackingAction:: methods=========
G4ClassificationOfNewTrack StackingAction::ClassifyNewTrack(const G4Track* aTrack) {
	if (aTrack->GetTrackID() <= PrimaryGeneratorAction::GetInstance()->GetNoOfParticles())
		return fUrgent;
	if (aTrack->GetParticleDefinition()->GetParticleName() == "opticalphoton") {
		if (aTrack->GetMomentumDirection().y() <= 0)
		//Those wouldn't be detected while they would increase the computation time significantly
		//Due to high refractive index, they would have bunch internal total reflection
			return fKill;
		else
		//The condition might be improved even further for even faster computations
		//(maybe in some subsequent version)
			return fUrgent;
	}
	//While the following return ain't perfect, additional particles could only raise the background.
	//Due to the geometry, the raise in peak would be insignificant (I believe).
	//Yet, those might increase the already long computation times
	return fKill;
}

endChR