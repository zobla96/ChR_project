//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

// user headers
#include "TrackingAction.hh"
// G4 headers
#include "G4ParticleGun.hh"

beginChR

//=========public ChR::TrackingAction:: methods=========

void TrackingAction::PreUserTrackingAction(const G4Track* aTrack) {
	if (aTrack->GetTrackID() <= g_primaryGenerator->GetParticleGun()->GetNumberOfParticles())
		m_specificTrackDataMap[aTrack->GetTrackID()] = SpecificTrackData{};
}

endChR