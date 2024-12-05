//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#include "SteppingAction.hpp"

beginChR

static std::mutex o_addingTime;

//=========public ChR::SteppingAction:: methods=========

SteppingAction::SteppingAction(const unsigned char val)
: m_verboseLevel(val) {
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
	if (aTrack->GetTrackID() <= g_primaryGenerator->GetNoOfParticles()) {
#ifdef boostEfficiency
		g_trackingAction->SetBeta(aTrack, (aStep->GetPreStepPoint()->GetBeta() + aStep->GetPostStepPoint()->GetBeta()) * 0.5);
		g_trackingAction->SetGlobalDirection(aTrack, aStep->GetDeltaPosition());
#endif // boostEfficiency
		if (g_detectorConstruction->GetNoOfRadLayers() <= 1)
			return;
		G4VPhysicalVolume* prePV = aStep->GetPreStepPoint()->GetPhysicalVolume();
		if (prePV->GetName() == "radiatorLayerPhys") {
			g_trackingAction->AddToDeltaEnergy(aTrack, aStep->GetPreStepPoint()->GetKineticEnergy() - aStep->GetPostStepPoint()->GetKineticEnergy());
			g_trackingAction->AddToStepLength(aTrack, aStep->GetStepLength());
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
				g_eventAction->AddToLayerDataVec((size_t/*CopyNo should be unsigned??*/)prePV->GetCopyNo(), g_trackingAction->GetDeltaEnergy(aTrack), g_trackingAction->GetStepLength(aTrack));
				g_trackingAction->SetDeltaEnergy(aTrack, 0.);
				g_trackingAction->SetStepLength(aTrack, 0.);
			}
			else /*(aStep->GetPostStepPoint()->GetPhysicalVolume()->GetName() == "worldPhys")*/
				g_eventAction->AddToLayerDataVec((size_t)prePV->GetCopyNo(), g_trackingAction->GetDeltaEnergy(aTrack), g_trackingAction->GetStepLength(aTrack));
		}
		return;
	}
	if (aTrack->GetParticleDefinition()->GetParticleName() == "opticalphoton") {
		/*std::cout << "FROM STEP\nx: " << aStep->GetPreStepPoint()->GetMomentumDirection().getX()
			<< ", y: " << aStep->GetPreStepPoint()->GetMomentumDirection().getY()
			<< ", z: " << aStep->GetPreStepPoint()->GetMomentumDirection().getZ() << '\n';
		std::cout << "phi: " << aStep->GetPreStepPoint()->GetMomentumDirection().getPhi() * 180 / CLHEP::pi << '\n'
			<< "theta: " << aStep->GetPreStepPoint()->GetMomentumDirection().getTheta() * 180 / CLHEP::pi << '\n';*/
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
					static std::atomic<int> noOfSideDet = 1;
					std::ostringstream err;
					int new_value = noOfSideDet.fetch_add(1, std::memory_order_relaxed);
					err << "Side detection #" << new_value << "\tz value: " << std::setprecision(11) << localCoords.getZ() << '\n';
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
					static std::atomic<int> counter = 1;
					int new_value = counter.fetch_add(1, std::memory_order_relaxed);
					if(new_value % 100 == 0)
						std::cout << "Detection #" << new_value << '\n';
				}
#ifdef followMinMaxValues
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
		}
	}
}

endChR