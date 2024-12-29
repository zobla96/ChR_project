//##########################################
//#######        VERSION 1.0.0       #######
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

#ifdef followMinMaxValues
static std::mutex o_addingTime;
#endif // followMinMaxValues
std::atomic<size_t> SteppingAction::m_noOfDetections = 1;

//=========public ChR::SteppingAction:: methods=========

SteppingAction::SteppingAction(const G4int verbose)
: m_verboseLevel(verbose) {
	p_steppingMessenger = new SteppingAction_Messenger{ this };
	G4VPhysicalVolume* detPhys = G4PhysicalVolumeStore::GetInstance()->GetVolume("detectorPhys", false);
	if (!detPhys) {
		G4Exception("SteppingAction::SteppingAction()", "FE_StepAction01", FatalException, "G4VPhysicalVolume instance \"detectorPhys\" not found!\n");
		return; //silence compiler C6011
	}
	m_rotToDetSystem = detPhys->GetObjectRotation()->inverse();
	m_trToDetSurfSystem = m_rotToDetSystem * detPhys->GetObjectTranslation();
	m_trToDetSurfSystem -= G4ThreeVector{ 0., 0., dynamic_cast<G4Tubs*>(detPhys->GetLogicalVolume()->GetSolid())->GetZHalfLength() };
}

SteppingAction::~SteppingAction() {
	delete p_steppingMessenger;
}

void SteppingAction::UserSteppingAction(const G4Step* aStep) {
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
	G4ThreeVector localCoords = m_rotToDetSystem * coords - m_trToDetSurfSystem;
	if (localCoords.getZ() > 1.e-10) { // IEEE754 standard
		if (m_verboseLevel > 1) {
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

	G4int new_value = m_noOfDetections.fetch_add(1, std::memory_order_relaxed);
	if (m_verboseLevel > 0 && new_value % 100 == 0)
		G4cout << "Detection #" << new_value << '\n';
#ifdef followMinMaxValues
	// check if some new limits are observed
	SpecificTrackData* theData = g_trackingAction->GetSpecificTrackData(aTrack);
	if (!theData)
		return;

	{
		std::lock_guard theLock{ o_addingTime };
		if (theData->m_phiValue != DBL_MAX) {
			if (g_maxPhiValue < theData->m_phiValue)
				g_maxPhiValue = theData->m_phiValue;
			if (g_minPhiValue > theData->m_phiValue)
				g_minPhiValue = theData->m_phiValue;
		}
		if (theData->m_thetaValue != DBL_MAX) {
			if (g_maxThetaValue < theData->m_thetaValue)
				g_maxThetaValue = theData->m_thetaValue;
			if (g_minThetaValue > theData->m_thetaValue)
				g_minThetaValue = theData->m_thetaValue;
		}
	}
#endif // followMinMaxValues
}

endChR