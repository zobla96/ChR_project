//##########################################
//#######        VERSION 1.1.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

// user headers
#include "SteppingAction.hh"
// G4 headers
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "G4Step.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4VProcess.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Tubs.hh"

beginChR

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifdef followMinMaxValues
static std::mutex o_addingTime;
#endif // followMinMaxValues
std::atomic<size_t> SteppingAction::fNoOfDetections = 1;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//=========public ChR::SteppingAction:: methods=========
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::SteppingAction(const G4int verbose)
: fSteppingMessenger(new SteppingAction_Messenger{ this }),
  fVerboseLevel(verbose)
{
  G4VPhysicalVolume* detPhys = G4PhysicalVolumeStore::GetInstance()->GetVolume("detectorPhys", false);
  if (!detPhys) {
    G4Exception("SteppingAction::SteppingAction()", "FE_StepAction01", FatalException, "G4VPhysicalVolume instance \"detectorPhys\" not found!\n");
    return; //silence compiler C6011
  }
  fRotToDetSystem = detPhys->GetObjectRotation()->inverse();
  fTrToDetSurfSystem = fRotToDetSystem * detPhys->GetObjectTranslation();
  fTrToDetSurfSystem -= G4ThreeVector{ 0., 0., dynamic_cast<G4Tubs*>(detPhys->GetLogicalVolume()->GetSolid())->GetZHalfLength() };
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction::~SteppingAction()
{
  delete fSteppingMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction::UserSteppingAction(const G4Step* aStep)
{
  G4Track* aTrack = aStep->GetTrack();
  if (aTrack->GetTrackID() <= g_primaryGenerator->GetParticleGun()->GetNumberOfParticles()) {
#ifdef boostEfficiency
    g_trackingAction->SetBeta(aTrack, (aStep->GetPreStepPoint()->GetBeta() + aStep->GetPostStepPoint()->GetBeta()) * 0.5);
    g_trackingAction->SetGlobalDirection(aTrack, aStep->GetDeltaPosition());
#endif // boostEfficiency
    return;
  }
#ifndef boostEfficiency // if it's a standard run without boosting efficiency
  if (aTrack->GetParticleDefinition()->GetParticleName() == "opticalphoton") {
    if (aTrack->GetMomentumDirection().y() <= 0)
      aTrack->SetTrackStatus(fStopAndKill);
  }
  else return; //not optical photon - this should never happen bcs StackingAction
#endif // !boostEfficiency
  if (aStep->GetPostStepPoint()->GetStepStatus() != fGeomBoundary)
    return;
  if (!(aStep->GetPreStepPoint()->GetPhysicalVolume()->GetName() == "worldPhys" &&
    aStep->GetPostStepPoint()->GetPhysicalVolume()->GetName() == "detectorPhys"))
    return;
  const G4ThreeVector& coords = aStep->GetPostStepPoint()->GetPosition();
  G4ThreeVector localCoords = fRotToDetSystem * coords - fTrToDetSurfSystem;
  if (localCoords.getZ() > 1.e-10) { // IEEE754 standard
    if (fVerboseLevel > 1) {
      static std::atomic<G4int> noOfSideDet = 1;
      std::ostringstream err;
      G4int new_value = noOfSideDet.fetch_add(1, std::memory_order_relaxed);
      err << "Side detection #" << new_value << "\tz value: " << std::setprecision(11) << localCoords.getZ() << '\n';
      G4Exception("SteppingAction::UserSteppingAction(...)", "WE_StepAction02", JustWarning, err);
    }
    return;
  }
  G4double waveLng = 1.239841984e-6 * m * eV / (aStep->GetPostStepPoint()->GetTotalEnergy() * nm);
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  // fill ntuples
  analysisManager->FillNtupleIColumn(0, G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID());
  analysisManager->FillNtupleDColumn(1, aStep->GetPostStepPoint()->GetTotalEnergy() / eV);
  analysisManager->FillNtupleDColumn(2, waveLng);
  analysisManager->FillNtupleDColumn(3, localCoords.getX() / um);
  analysisManager->FillNtupleDColumn(4, localCoords.getY() / um);
  analysisManager->AddNtupleRow();
  // fill H1s
  for (G4int i = 0; i < analysisManager->GetNofH1s(); i++)
    analysisManager->FillH1(i, waveLng);

  G4int new_value = fNoOfDetections.fetch_add(1, std::memory_order_relaxed);
  if (fVerboseLevel > 0 && new_value % 100 == 0)
    G4cout << "Detection #" << new_value << '\n';
#ifdef followMinMaxValues
  // check if some new limits are observed
  SpecificTrackData* theData = g_trackingAction->GetSpecificTrackData(aTrack);
  if (!theData)
    return;

  {
    std::lock_guard theLock{ o_addingTime };
    if (theData->fPhiValue != DBL_MAX) {
      if (g_maxPhiValue < theData->fPhiValue)
        g_maxPhiValue = theData->fPhiValue;
      if (g_minPhiValue > theData->fPhiValue)
        g_minPhiValue = theData->fPhiValue;
    }
    if (theData->fThetaValue != DBL_MAX) {
      if (g_maxThetaValue < theData->fThetaValue)
        g_maxThetaValue = theData->fThetaValue;
      if (g_minThetaValue > theData->fThetaValue)
        g_minThetaValue = theData->fThetaValue;
    }
  }
#endif // followMinMaxValues
}

endChR