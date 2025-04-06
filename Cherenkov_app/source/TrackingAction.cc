//##########################################
//#######        VERSION 1.1.0       #######
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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//=========public ChR::TrackingAction:: methods=========
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void TrackingAction::PreUserTrackingAction(const G4Track* aTrack)
{
  if (aTrack->GetTrackID() <= g_primaryGenerator->GetParticleGun()->GetNumberOfParticles())
    fSpecificTrackDataMap[aTrack->GetTrackID()] = SpecificTrackData{};
}

endChR