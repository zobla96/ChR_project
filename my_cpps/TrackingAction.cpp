#include "TrackingAction.hpp"

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

beginChR

//=========public ChR::TrackingAction:: methods=========
TrackingAction::TrackingAction()
: r_noOfPrimaries(PrimaryGeneratorAction::GetInstance()->GetRefNoOfParticles()) {

}

void TrackingAction::PreUserTrackingAction(const G4Track* aTrack) {
	if(aTrack->GetTrackID() <= r_noOfPrimaries)
		m_layerData[aTrack->GetTrackID()];
}

void TrackingAction::PostUserTrackingAction(const G4Track* aTrack) {
	if (aTrack->GetTrackID() <= r_noOfPrimaries)
		m_layerData.erase(aTrack->GetTrackID());
}

endChR