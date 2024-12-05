//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#include "TrackingAction.hpp"

beginChR

//=========public ChR::TrackingAction:: methods=========

void TrackingAction::PreUserTrackingAction(const G4Track* aTrack) {
	if (aTrack->GetTrackID() <= g_primaryGenerator->GetNoOfParticles())
		m_specificTrackDataMap[aTrack->GetTrackID()] = SpecificTrackData{};
}

endChR