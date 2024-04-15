#pragma once
#ifndef CherenkovProcess_Messenger_hpp
#define CherenkovProcess_Messenger_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "CherenkovProcess.hpp"
//G4 headers
#include "G4UImessenger.hh"
//...
#include "G4UIdirectory.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4ParticleTable.hh"
#include "G4MTRunManager.hh"
//std:: headers

beginChR

template<typename T>
class CherenkovProcess;

template <typename T>
class CherenkovProcess_Messenger : public G4UImessenger {
public:
	CherenkovProcess_Messenger(CherenkovProcess<T>*);
	virtual ~CherenkovProcess_Messenger() override;
	virtual void SetNewValue(G4UIcommand*, G4String) override;
private:
	CherenkovProcess<T>* p_ChRProcess = nullptr;
	//2x DIR
	G4UIdirectory* p_ChRMessengerDir = nullptr;
	G4UIdirectory* p_BaseChRModelUIDirectory = nullptr;
	//ChR commands
	G4UIcmdWithABool* p_processFlag = nullptr;
	G4UIcommand* p_dumpInfo = nullptr;
	G4UIcommand* p_processDescription = nullptr;
	//BaseChR_Model commands
	G4UIcmdWithABool* p_useEnergyLossInModels = nullptr;
	G4UIcmdWithAnInteger* p_noOfBetaSteps = nullptr;
	G4UIcmdWithAnInteger* p_modelVerboseLevel = nullptr;
	G4UIcommand* p_printPhysicsVector = nullptr;
};

template<typename T>
CherenkovProcess_Messenger<T>::CherenkovProcess_Messenger(CherenkovProcess<T>* theChRProcess)
:p_ChRProcess(theChRProcess) {
	//DIR #1
	p_ChRMessengerDir = new G4UIdirectory{ "/ChR_project/ChRProcess/" };
	p_ChRMessengerDir->SetGuidance("All commands related to Cherenkov process");
	//ChR commands
	p_processFlag = new G4UIcmdWithABool{ "/ChR_project/ChRProcess/processFlag", this };
	p_processFlag->SetGuidance("Used to change or clear Cherenkov process flag.");
	p_processFlag->SetGuidance("If this flag is set to 'true', Cherenkov process is executed (unless exception is thrown).");
	p_processFlag->SetGuidance("If this flag is set to 'false', Cherenkov process won't be executed!");
	p_processFlag->SetGuidance("NOTE: if CherenkovProcess<ChRModelIndex> is used, this flag is useless!");
	p_processFlag->SetParameterName("ChR_flag", true);
	p_processFlag->SetDefaultValue(true);
	p_processFlag->AvailableForStates(G4State_Idle); //might add later phases as well, but those can't be executed from .mac

	p_dumpInfo = new G4UIcommand{ "/ChR_project/ChRProcess/dumpInfo", this };
	p_dumpInfo->SetGuidance("Used to print all available information about Cherenkov process class and registered models.");
	p_dumpInfo->SetToBeBroadcasted(false);
	p_dumpInfo->AvailableForStates(G4State_Idle);

	p_processDescription = new G4UIcommand{ "/ChR_project/ChRProcess/processDescription", this };
	p_processDescription->SetGuidance("Used to print about what Cherenkov process is.");
	p_processDescription->SetToBeBroadcasted(false);
	p_processDescription->AvailableForStates(G4State_Idle);

	//DIR #2
	p_BaseChRModelUIDirectory = new G4UIdirectory{ "/ChR_project/BaseChR_Model/" };
	p_BaseChRModelUIDirectory->SetGuidance("All commands related to BaseChR_Model");
	//BaseChR_Model commands
	p_useEnergyLossInModels = new G4UIcmdWithABool{ "/ChR_project/BaseChR_Model/useEnergyLossInModels", this };
	p_useEnergyLossInModels->SetGuidance("Used to activate energy conservation law for all registered models.");
	p_useEnergyLossInModels->SetGuidance("According to G4Cerenkov and Tamm's theory, Cherenkov radiation doesn't change charged particle's energy.");
	p_useEnergyLossInModels->SetGuidance("However, if you select \"true\" in new models, the change in energy is included.");
	p_useEnergyLossInModels->SetGuidance("Note: in the current version, only energy is changed, but the direction of the particle is not!");
	p_useEnergyLossInModels->SetParameterName("ChR_data", true);
	p_useEnergyLossInModels->SetDefaultValue(false);
	p_useEnergyLossInModels->AvailableForStates(G4State_Idle);

	p_noOfBetaSteps = new G4UIcmdWithAnInteger{ "/ChR_project/BaseChR_Model/noOfBetaSteps", this };
	p_noOfBetaSteps->SetGuidance("Used to change the number of beta steps (betaNodes == betaStep + 1).");
	p_noOfBetaSteps->SetGuidance("When building physics tables for BaseChR_Model, the critical energies are considered through the relativistic velocity \"beta\" of the charged particle.");
	p_noOfBetaSteps->SetGuidance("For critical energies, the physics tables are divided into steps from beta minimal to beta maximal.");
	p_noOfBetaSteps->SetParameterName("ChR_data", true);
	p_noOfBetaSteps->SetDefaultValue(20);
	p_noOfBetaSteps->SetRange("ChR_data>1 && ChR_data<=255");
	p_noOfBetaSteps->AvailableForStates(G4State_Idle);

	p_modelVerboseLevel = new G4UIcmdWithAnInteger{ "/ChR_project/BaseChR_Model/changeModelVerbose", this };
	p_modelVerboseLevel->SetGuidance("Used to change the verbose level for all registered models.");
	p_modelVerboseLevel->SetGuidance("NOTE: it's almost useless right now as I still almost never used it.");
	p_modelVerboseLevel->SetParameterName("verboseLevel", true);
	p_modelVerboseLevel->SetDefaultValue(1);
	p_modelVerboseLevel->SetRange("verboseLevel>=0 && verboseLevel<256");
	p_modelVerboseLevel->AvailableForStates(G4State_Idle);

	p_printPhysicsVector = new G4UIcommand("/ChR_project/BaseChR_Model/printBaseChRPhysicsVector", this);
	p_printPhysicsVector->SetGuidance("Used to print the loaded static physics vector of BaseChR_model.");
	p_printPhysicsVector->SetToBeBroadcasted(false);
	p_printPhysicsVector->AvailableForStates(G4State_Idle);
}

template<typename T>
CherenkovProcess_Messenger<T>::~CherenkovProcess_Messenger() {
	//2x DIR
	delete p_ChRMessengerDir;
	delete p_BaseChRModelUIDirectory;
	//ChR commands
	delete p_processFlag;
	delete p_dumpInfo;
	delete p_processDescription;
	//BaseChR_Model commands
	delete p_useEnergyLossInModels;
	delete p_noOfBetaSteps;
	delete p_modelVerboseLevel;
	delete p_printPhysicsVector;
}

template<typename T>
void CherenkovProcess_Messenger<T>::SetNewValue(G4UIcommand* uiCmd, G4String aStr) {
	if (uiCmd == p_processFlag) {
		if (p_processFlag->ConvertToBool(aStr))
			p_ChRProcess->ClearFlag();
	}
	else if (uiCmd == p_dumpInfo) {
		p_ChRProcess->DumpInfo();
	}
	else if (uiCmd == p_processDescription) {
		p_ChRProcess->ProcessDescription();
	}
	else if (uiCmd == p_useEnergyLossInModels) {
		for (auto& [key, value] : p_ChRProcess->GetChRRegisteredModels())
			value->SetUseModelWithEnergyLoss(p_useEnergyLossInModels->ConvertToBool(aStr));
	}
	else if (uiCmd == p_noOfBetaSteps) {
		const unsigned char newBetaStep = (const unsigned char)p_noOfBetaSteps->ConvertToInt(aStr);
		if (newBetaStep == p_ChRProcess->GetChRRegisteredModels().begin()->second->GetNoOfBetaSteps()) {
			const char* msg = "betaStep of Cherenkov models has not been changed - you used the same number that's already set!\n";
			G4Exception("CherenkovProcess_Messenger<T>::SetNewValue", "WE1015", JustWarning, msg);
			return;
		}
		for (auto& [key, value] : p_ChRProcess->GetChRRegisteredModels())
			value->SetNoOfBetaSteps(newBetaStep);
		if (G4MTRunManager::GetMasterThreadId() != std::this_thread::get_id())
			return;
		std::cout << "All betaStep values have been changed! Now deleting old physics tables...\n";
		p_ChRProcess->GetChRRegisteredModels().begin()->second->GetChRPhysDataVec().clear();
		std::cout << "Old physics tables have been removed! Now creating new physics tables...\n";
		auto* particleIterator = G4ParticleTable::GetParticleTable()->GetIterator();
		particleIterator->reset();
		while ((*particleIterator)()) { //a useless and bad loop for this process, but still... pre-run time
			p_ChRProcess->GetChRRegisteredModels().begin()->second->BuildModelPhysicsTable(*(particleIterator->value()));
		}
		std::cout << "Physics tables have been successfully rebuilt!\n";
	}
	else if (uiCmd == p_modelVerboseLevel) {
		for (auto& [key, value] : p_ChRProcess->GetChRRegisteredModels())
			value->SetVerboseLevel((const unsigned char)p_modelVerboseLevel->ConvertToInt(aStr));
	}
	else if (uiCmd == p_printPhysicsVector) {
		p_ChRProcess->GetChRRegisteredModels().begin()->second->PrintChRPhysDataVec();
	}
	else
		G4Exception("CherenkovProcess_Messenger<T>::SetNewValue", "WE1016", JustWarning, "Command not found!\n");
}

endChR

#endif // !CherenkovProcess_Messenger_hpp