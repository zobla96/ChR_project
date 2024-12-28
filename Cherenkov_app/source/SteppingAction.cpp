//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#include "SteppingAction.hpp"

beginChR

//=========public ChR::SteppingAction:: methods=========

SteppingAction::SteppingAction(EventAction* evAction, TrackingAction* trAction, const unsigned char val)
: p_trackingAction(trAction), p_eventAction(evAction),
r_noOfRadLayers(DetectorConstruction::GetInstance()->GetRefNoOfRadLayers()),
r_noOfPrimaries(PrimaryGeneratorAction::GetInstance()->GetRefNoOfParticles()),
m_verboseLevel(val) {
	p_steppingMessenger = new SteppingAction_Messenger{ this };
	G4VPhysicalVolume* detPhys = nullptr;
	for (auto* i : *(G4PhysicalVolumeStore::GetInstance())) {
		if (i->GetName() == "detectorPhys") {
			detPhys = i;
			break;
		}
	}
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
	//NOTE: the following isn't important for obtaining Cherenkov spectral lines, but can be
	//helpful if one want to obtain a function of energy loss per penetration depth
	if (r_noOfRadLayers > 1 && aTrack->GetTrackID() <= (int)r_noOfPrimaries) {
		G4VPhysicalVolume* prePV = aStep->GetPreStepPoint()->GetPhysicalVolume();
		if (prePV->GetName() == "radiatorLayerPhys") {
			p_trackingAction->AddToLayerDeltaEnergy(aTrack, aStep->GetPreStepPoint()->GetKineticEnergy() - aStep->GetPostStepPoint()->GetKineticEnergy());
			p_trackingAction->AddToLayerStep(aTrack, aStep->GetStepLength());
		}
		else return;
		if (aStep->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) {
			if (aStep->GetPostStepPoint()->GetPhysicalVolume()->GetName() == "radiatorLayerPhys") {
				if (aStep->GetPostStepPoint()->GetMomentumDirection().getZ() < 0) {
					G4Exception("SteppingAction::UserSteppingAction", "WE_StepAction01", JustWarning, "A primary moving backwards!\n");
					//this should never happen for the used energies, I believe
					aTrack->SetTrackStatus(fStopAndKill);
					return;
				}
				//no need for safeties for our energies
				p_eventAction->AddToLayerDataVec((size_t/*CopyNo should be unsigned??*/)prePV->GetCopyNo(), p_trackingAction->GetLayerDeltaEnergy(aTrack), p_trackingAction->GetLayerStep(aTrack));
				p_trackingAction->SetLayerDeltaEnergy(aTrack, 0.);
				p_trackingAction->SetLayerStep(aTrack, 0.);
			}
			else /*(aStep->GetPostStepPoint()->GetPhysicalVolume()->GetName() == "worldPhys")*/
				p_eventAction->AddToLayerDataVec((size_t)prePV->GetCopyNo(), p_trackingAction->GetLayerDeltaEnergy(aTrack), p_trackingAction->GetLayerStep(aTrack));
		}
		return;
	}
	if (aTrack->GetParticleDefinition()->GetParticleName() == "opticalphoton") {
		if (aTrack->GetMomentumDirection().y() <= 0)
			aTrack->SetTrackStatus(fStopAndKill);
	}
	else return; //not optical photon - this should never happen bcs StackingAction
	if (aStep->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) {
		if (aStep->GetPreStepPoint()->GetPhysicalVolume()->GetName() == "worldPhys" &&
			aStep->GetPostStepPoint()->GetPhysicalVolume()->GetName() == "detectorPhys") {
			const G4ThreeVector& coords = aStep->GetPostStepPoint()->GetPosition();
			G4ThreeVector localCoords = m_rotToDetSystem * coords - m_trToDetSurfSystem;
			if (localCoords.getZ() > 1.e-10) { //CARE HERE! IEEE754 standard; don't care about negative values here
				if (m_verboseLevel > 1) {
					static std::atomic<int> noOfSideDet = 0;
					std::ostringstream err;
					err << "Side detection #" << ++noOfSideDet << "\tz value: " << std::setprecision(11) << localCoords.getZ() << '\n';
					G4Exception("SteppingAction::UserSteppingAction(...)", "WE_StepAction02", JustWarning, err);
				}
			}
			else {
				G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
				analysisManager->FillNtupleIColumn(0, G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID());
				analysisManager->FillNtupleDColumn(1, aStep->GetPostStepPoint()->GetTotalEnergy() / eV);
				analysisManager->FillNtupleDColumn(2, 1.239841984e-6 * m * eV / (aStep->GetPostStepPoint()->GetTotalEnergy() * nm));
				analysisManager->FillNtupleDColumn(3, localCoords.getX() / um);
				analysisManager->FillNtupleDColumn(4, localCoords.getY() / um);
				analysisManager->AddNtupleRow();
				if (m_verboseLevel > 0) {
					static std::atomic<int> counter = 0;
					std::cout << "Event #" << ++counter << '\n' << "Wl: " << 1.239841984e-6 * m * eV / (aStep->GetPostStepPoint()->GetTotalEnergy() * nm) << '\n';
				}
			}
		}
	}
}

endChR