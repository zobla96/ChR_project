//##########################################
//#######        VERSION 1.1.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

// user headers
#include "StackingAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"
#include "TrackingAction.hh"
#include "PhysicsList.hh"
// G4 headers (from ChR_process_lib)
#include "G4ExtraOpticalParameters.hh"
// ...
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"

beginChR

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//=========public ChR::StackingAction:: methods=========
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StackingAction::StackingAction()
#ifdef boostEfficiency
: fRotationMatrix(G4PhysicalVolumeStore::GetInstance()->GetVolume("radiatorPhys")->GetRotation()->inverse()),
  fDeltaPhi(180.),
  fThetaMin(-DBL_MAX),
  fThetaMax(DBL_MAX),
  fWithGaussSigma(false)
{
  // taking inverse of the matrix as the G4RotationMatrix implements active transforms
  fRindexVector = G4Material::GetMaterial(g_detectorConstruction->GetRadiatorMaterialName())->
    GetMaterialPropertiesTable()->GetProperty(kRINDEX);
  if (dynamic_cast<const PhysicsList*>(G4RunManager::GetRunManager()->GetUserPhysicsList())->GetOpticalPhysicsInUse() != UseOptical::G4OpticalPhysics_option2)
    return;
  if (G4ExtraOpticalParameters::GetInstance()->FindChRMatData(
    G4LogicalVolumeStore::GetInstance()->GetVolume("radiatorLogic"))->m_executeModel == 1)
    fWithGaussSigma = true;
}
#else
{}
#endif // boostEfficiency

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ClassificationOfNewTrack StackingAction::ClassifyNewTrack(const G4Track* aTrack)
{
  if (aTrack->GetTrackID() <= g_primaryGenerator->GetParticleGun()->GetNumberOfParticles())
    return fUrgent;
#ifdef standardRun
 #ifdef boostEfficiency
  /*
  The following is just a simple and quick solution to significantly increase the efficiency of simulations.
  The idea of the experiment is to use a very small solid detector angle. That also implies that most of
  Cherenkov photons fly around without being detected! Because of that, the idea here is to direct all emitted
  photons towards the detector without violating Monte Carlo principles. Still, this can be done even better
  and one can even determine the expected angles, wavelengths, etc., in the run time.
  As this code cannot adapt the conditions in run time, one will get a fatal exception (it can and should
  be turned off to change the default geometry - it's done to make sure the user notices and understands
  the problem). Anyhow, all member variables of this class are set in the beginning of each run (in RunAction),
  and if the user wants (it should be done for non-default runs) to boost efficiency, those values should be
  changed. For now, this is done manually and might be improved in the future.
  See the NOTE below.
  */
  if (aTrack->GetParticleDefinition()->GetParticleName() != "opticalphoton")
    return fKill;
  G4ThreeVector& nonConstPhotonDirection = const_cast<G4ThreeVector&>(aTrack->GetMomentumDirection());
  G4ThreeVector& nonConstPolarization = const_cast<G4ThreeVector&>(aTrack->GetPolarization());
  const SpecificTrackData* parentData = g_trackingAction->GetSpecificTrackData(aTrack->GetParentID());
  G4double newPhi = (90 + (G4UniformRand() - 0.5) * fDeltaPhi) * deg;
  G4double sinNewPhi = std::sin(newPhi);
  G4double cosNewPhi = std::cos(newPhi);
  if (!fWithGaussSigma) {
    G4double cosTheta = parentData->fGlobalDirection.dot(nonConstPhotonDirection);
    G4double sinTheta = std::sqrt((1. - cosTheta) * (1 + cosTheta));
    nonConstPhotonDirection.set(sinTheta * cosNewPhi, sinTheta * sinNewPhi, cosTheta);
    nonConstPhotonDirection.rotateUz(parentData->fGlobalDirection);
    nonConstPolarization.set(cosTheta * cosNewPhi, cosTheta * sinNewPhi, -sinTheta);
    nonConstPolarization.rotateUz(parentData->fGlobalDirection);
#ifdef followMinMaxValues
    SpecificTrackData& theData = g_trackingAction->GetOrCreateSpecificTrackData(aTrack);
    // to remove values from previous runs (if I'm not mistaken, G4TrackingAction is not created/destroyed all the time)
    theData = SpecificTrackData{};
    theData.fPhiValue = newPhi;
#endif // followMinMaxValues
    return fUrgent;
  }
  G4double sampledRI = fRindexVector->Value(aTrack->GetTotalEnergy());
  G4double waveLng = 1.239841984e-6 * m * eV / aTrack->GetTotalEnergy();
  // for some reason I cannot find the direct way to obtain parent's track (e.g., through the id)!
  // that's why I saved some data in the tracking action. Note that the step can be used this way
  // because Cherenkov process sets the flag "track secondaries first"... that means that the will
  // not be removed before the secondary photons are processed! Otherwise, I would need to take a
  // more complex approach.
  G4double theAngleCos = parentData->fGlobalDirection.dot(fRotationMatrix(G4ThreeVector{ 0., 0., 1. }));
  if (theAngleCos == 0.)
    return fKill;
  G4double thetaChR = std::acos(1. / (sampledRI * parentData->fBeta));
  // NOTE
  // It is very important to keep in mind that this equation gives good results for angles ~90 deg.
  // If the detector's solid angle is increased, it's very possible that one will need to change to
  // the more general approach, similar to the one in G4ThinTargetChR_Model
  G4double gaussSigma = 0.42466 * waveLng * theAngleCos / (2 * sampledRI * g_detectorConstruction->GetRadiatorThickness() *
    (std::tan(thetaChR - g_detectorConstruction->GetRadiatorAngle()) +
      std::tan(g_detectorConstruction->GetRadiatorAngle())));

  thetaChR = G4RandGauss::shoot(thetaChR, gaussSigma);
  if (thetaChR > fThetaMax || thetaChR < fThetaMin) // some 20-25% boost in time
    return fKill;
  G4double cosTheta = std::cos(thetaChR);
  G4double sinTheta = std::sin(thetaChR);
  nonConstPhotonDirection.set(sinTheta * cosNewPhi, sinTheta * sinNewPhi, cosTheta);
  nonConstPhotonDirection.rotateUz(parentData->fGlobalDirection);
  nonConstPolarization.set(cosTheta * cosNewPhi, cosTheta * sinNewPhi, -sinTheta);
  nonConstPolarization.rotateUz(parentData->fGlobalDirection);
#ifdef followMinMaxValues
  SpecificTrackData& theData = g_trackingAction->GetOrCreateSpecificTrackData(aTrack);
  // to remove values from previous runs (for the photon of ID)
  theData = SpecificTrackData{};
  theData.fPhiValue = newPhi;
  theData.fThetaValue = thetaChR;
#endif // followMinMaxValues
  return fUrgent;

 #else // !biistEfficiency
  if (aTrack->GetParticleDefinition()->GetParticleName() == "opticalphoton") {
    if (aTrack->GetMomentumDirection().y() <= 0)
      // Those wouldn't be detected while they would increase the computation time significantly
      // Due to high refractive index, they would experience total internal reflections
      return fKill;
    else
      return fUrgent;
  }
  // While the following return ain't perfect, additional particles could only raise the background.
  // Due to the geometry, the raise in peaks' intensity would be insignificant (I believe).
  // Yet, those might increase the already long computation times
  return fKill;
 #endif // boostEfficiency
#else // !standardRun
 #ifdef captureChRPhotonEnergyDistribution
  if (aTrack->GetParticleDefinition()->GetParticleName() == "opticalphoton") {
    /*
    for checking performance, undef "captureChRPhotonEnergyDistribution".
    It significantly slows down a multi-thread app, as if some
    atomics are a problem (haven't analyzed). The previous "as if" is
    because I got better performance on ~4-5 threads compared to 20 (CPU
    has 20 threads) which can be related to atomics' cache memory effects!
    */
    G4double energy = aTrack->GetTotalEnergy();
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    analysisManager->FillNtupleIColumn(0, G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID());
    analysisManager->FillNtupleDColumn(1, energy / eV);
    analysisManager->FillNtupleDColumn(2, 1.239841984e-6 * m * eV / (energy * nm));
    analysisManager->AddNtupleRow();
  }
 #endif // captureChRPhotonEnergyDistribution
  return fKill;
#endif // standardRun
}

endChR