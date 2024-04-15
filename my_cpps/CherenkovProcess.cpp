#include "CherenkovProcess.hpp"

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

beginChR

//=========full explicit specializations for ChR::CherenkovProcess<ChRModelIndex>::=========
template<>
CherenkovProcess<ChRModelIndex>::CherenkovProcess(std::string name)
:G4VDiscreteProcess(name, fElectromagnetic),
m_processFlag(true) {
	p_ChRProcessMessenger = new CherenkovProcess_Messenger<ChRModelIndex>{ this };
	if (AddNewChRModel(ChRModelIndex::AlmostOriginalChR, new AlmostOriginalChR_Model{}) && verboseLevel > 0)
		std::cout << "AlmostOriginalChR_Model has been added to CherenkovProcess\n";
	if (AddNewChRModel(ChRModelIndex::TammThinTargetChR, new TammThinTarget_Model<ChRModelIndex>{}) && verboseLevel > 0)
		std::cout << "TammThinTarget_Model has been added to CherenkovProcess\n";
}

template<>
double CherenkovProcess<ChRModelIndex>::PostStepGetPhysicalInteractionLength(const G4Track& aTrack, double previousStepSize, G4ForceCondition* aCondition) {
	m_selectedModel = nullptr;
	const auto myParameters = MyOpticalParameters<ChRModelIndex>::GetInstance();
	G4LogicalVolume* currentLV = aTrack.GetVolume()->GetLogicalVolume();
	CherenkovMatData<ChRModelIndex>* matData = myParameters->FindChRMatData(currentLV);
	//maybe could've used the previous as arguemnts, but still...
	if (matData->m_forceModel == ChRModelIndex::Default || matData->m_forceModel == ChRModelIndex::AlmostOriginalChR) {
		m_selectedModel = CheckIfAModelIsRegistered(ChRModelIndex::AlmostOriginalChR);
		return m_selectedModel->PostStepModelIntLength(aTrack, previousStepSize, aCondition);
	}
	else {
		m_selectedModel = CheckIfAModelIsRegistered(matData->m_forceModel);
		return m_selectedModel->PostStepModelIntLength(aTrack, previousStepSize, aCondition);
	}
}

template<>
G4VParticleChange* CherenkovProcess<ChRModelIndex>::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) {
	return m_selectedModel->PostStepModelDoIt(aTrack, aStep);
}

template<>
void CherenkovProcess<ChRModelIndex>::StartTracking(G4Track* aTrack) {
	G4VProcess::StartTracking(aTrack);
}

endChR