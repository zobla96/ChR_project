//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//G4 headers
#include "G4CherenkovProcess_Messenger.hh"
#include "G4CherenkovProcess.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIparameter.hh"
#include "G4ParticleTable.hh"
#include "G4MTRunManager.hh"
#include "G4SystemOfUnits.hh"
//std:: headers
#include "sstream"

//=========public G4CherenkovProcess_Messenger:: methods=========

G4CherenkovProcess_Messenger::G4CherenkovProcess_Messenger(G4CherenkovProcess* theChRProcess)
: p_ChRProcess(theChRProcess) {

	G4UIparameter* uiParameter = nullptr;

	//DIR #1
	p_ChRMessengerDir = new G4UIdirectory{ "/process/optical/G4ChRProcess/" };
	p_ChRMessengerDir->SetGuidance("All commands related to G4CherenkovProcess");
	//ChR commands
	p_dumpChRInfo = new G4UIcommand{ "/process/optical/G4ChRProcess/dumpInfo", this };
	p_dumpChRInfo->SetGuidance("Used to print all available information about Cherenkov process class and registered models.");
	p_dumpChRInfo->SetToBeBroadcasted(false);
	p_dumpChRInfo->AvailableForStates(G4State_Idle);

	p_processDescription = new G4UIcommand{ "/process/optical/G4ChRProcess/processDescription", this };
	p_processDescription->SetGuidance("Used to print about what Cherenkov process is.");
	p_processDescription->SetToBeBroadcasted(false);
	p_processDescription->AvailableForStates(G4State_Idle);

	p_isApplicable = new G4UIcmdWithAString{ "/process/optical/G4ChRProcess/isApplicable", this };
	p_isApplicable->SetGuidance("Check if a specific particle can generate Cherenkov photons.");
	p_isApplicable->SetGuidance("NOTE: Make sure to write a particle name correctly.");
	p_isApplicable->SetParameterName("particleName", false);
	p_isApplicable->SetToBeBroadcasted(false);
	p_isApplicable->AvailableForStates(G4State_Idle);
	
	p_minEnergy = new G4UIcommand{ "/process/optical/G4ChRProcess/minEnergy",this };
	p_minEnergy->SetGuidance("Check if a specific particle can generate Cherenkov photons");
	p_minEnergy->SetGuidance("for a specific material, and print the minimal energy");
	p_minEnergy->SetGuidance("needed for Cherenkov photons' generation.");
	p_minEnergy->SetGuidance("You need to provide two strings:");
	p_minEnergy->SetGuidance("1. A name of the particle");
	p_minEnergy->SetGuidance("2. A name of the registered material");
	//G4UIParameter objects are deleted in deconstructor of G4UIcommand
	uiParameter = new G4UIparameter{ "particleName", 's', false };
	p_minEnergy->SetParameter(uiParameter);
	uiParameter = new G4UIparameter{ "materialName", 's', false };
	p_minEnergy->SetParameter(uiParameter);
	p_minEnergy->SetToBeBroadcasted(false);
	p_minEnergy->AvailableForStates(G4State_Idle);

	//DIR #2
	p_BaseChRModelUIDirectory = new G4UIdirectory{ "/process/optical/G4ChRProcess/Models/" };
	p_BaseChRModelUIDirectory->SetGuidance("All commands related to registered models in G4CherenkovProcess");
	//BaseChR_Model commands
	p_useEnergyLossInModels = new G4UIcmdWithABool{ "/process/optical/G4ChRProcess/Models/useEnergyLossInModels", this };
	p_useEnergyLossInModels->SetGuidance("Used to activate energy conservation law for all registered models.");
	p_useEnergyLossInModels->SetGuidance("According to G4Cerenkov and Frank-Tamm theory, Cherenkov radiation doesn't change charged particle's energy.");
	p_useEnergyLossInModels->SetGuidance("However, if you select \"true\" in new models, the energy change is considered (note that it's minimal anyway).");
	p_useEnergyLossInModels->SetParameterName("changeEnergyInChR", true);
	p_useEnergyLossInModels->SetDefaultValue(true);
	p_useEnergyLossInModels->SetToBeBroadcasted(true);
	p_useEnergyLossInModels->AvailableForStates(G4State_Idle);

	p_noOfBetaSteps = new G4UIcmdWithAnInteger{ "/process/optical/G4ChRProcess/Models/noOfBetaSteps", this };
	p_noOfBetaSteps->SetGuidance("Used to change the number of beta steps (betaNodes == betaSteps + 1).");
	p_noOfBetaSteps->SetGuidance("When building physics tables for BaseChR_Model, the critical energies are considered through the relativistic velocity \"beta\" of the charged particle.");
	p_noOfBetaSteps->SetGuidance("For critical energies, the physics tables are divided into steps from minimal beta to maximal beta.");
	p_noOfBetaSteps->SetParameterName("betaSteps", false);
	p_noOfBetaSteps->SetRange("betaSteps>=1");
	p_noOfBetaSteps->SetToBeBroadcasted(false);
	p_noOfBetaSteps->AvailableForStates(G4State_Idle);

	p_modelVerboseLevel = new G4UIcmdWithAnInteger{ "/process/optical/G4ChRProcess/Models/changeModelVerbose", this };
	p_modelVerboseLevel->SetGuidance("Used to change the verbose level for all registered models.");
	p_modelVerboseLevel->SetParameterName("verboseLevel", true);
	p_modelVerboseLevel->SetDefaultValue(1);
	p_modelVerboseLevel->SetRange("verboseLevel>=0 && verboseLevel<256");
	p_modelVerboseLevel->SetToBeBroadcasted(true);
	p_modelVerboseLevel->AvailableForStates(G4State_Idle);

	p_printPhysicsVector = new G4UIcommand{ "/process/optical/G4ChRProcess/Models/printBaseChRPhysicsVector", this };
	p_printPhysicsVector->SetGuidance("Used to print the loaded static physics vector of BaseChR_model.");
	p_printPhysicsVector->SetGuidance("Used to print the loaded static physics vector of G4StandardChRProcess.");
	p_printPhysicsVector->SetGuidance("printLevel == 0 -> print only basic available information about registered physics tables");
	p_printPhysicsVector->SetGuidance("printLevel == 1 -> print standard + exotic RIndex CDF values");
	p_printPhysicsVector->SetGuidance("printLevel >= 2 -> print all available information about registered physics tables");
	p_printPhysicsVector->SetGuidance("materialName - omitted -> prints physics tables for all registered materials");
	p_printPhysicsVector->SetGuidance("materialName - provided -> prints physics tables for materialName");
	uiParameter = new G4UIparameter{ "printLevel", 'i', true };
	uiParameter->SetParameterRange("printLevel>=0 && printLevel<256");
	p_printPhysicsVector->SetParameter(uiParameter);
	uiParameter = new G4UIparameter{ "materialName", 's', true };
	p_printPhysicsVector->SetParameter(uiParameter);
	p_printPhysicsVector->SetToBeBroadcasted(false);
	p_printPhysicsVector->AvailableForStates(G4State_Idle);
}

G4CherenkovProcess_Messenger::~G4CherenkovProcess_Messenger() {
	//2x DIR
	delete p_ChRMessengerDir;
	delete p_BaseChRModelUIDirectory;
	//G4CherenkovProcess commands
	delete p_dumpChRInfo;
	delete p_processDescription;
	delete p_isApplicable;
	delete p_minEnergy;
	//G4BaseChR_Model commands
	delete p_useEnergyLossInModels;
	delete p_noOfBetaSteps;
	delete p_modelVerboseLevel;
	delete p_printPhysicsVector;
}

void G4CherenkovProcess_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr) {
	if (uiCmd == p_dumpChRInfo) {
		p_ChRProcess->DumpInfo();
	}
	else if (uiCmd == p_processDescription) {
		p_ChRProcess->ProcessDescription();
	}
	else if (uiCmd == p_isApplicable) {
		G4ParticleTable::G4PTblDicIterator* itr = G4ParticleTable::GetParticleTable()->GetIterator();
		itr->reset();
		G4bool foundIt = false;
		while ((*itr)()) {
			if (itr->value()->GetParticleName() == aStr) {
				foundIt = true;
				break;
			}
		}
		if (foundIt) {
			if (p_ChRProcess->IsApplicable(*(itr->value())))
				std::cout << "Particle " << aStr << " can generate Cherenkov photons!\n";
			else
				std::cout << "Particle " << aStr << " cannot generate Cherenkov photons!\n";
		}
		else {
			std::ostringstream err;
			err << "There is no registered particle under the name " << std::quoted(aStr) << '\n';
			G4Exception("G4CherenkovProcess_Messenger::SetNewValue", "WE_ChRMessenger01", JustWarning, err);
		}
	}
	else if (uiCmd == p_minEnergy) {
		std::string particleName, materialName;
		std::string::iterator spaceChar = std::find(aStr.begin(), aStr.end(), ' ');
		std::copy(aStr.begin(), spaceChar, std::back_inserter(particleName));
		std::copy(spaceChar + 1, aStr.end(), std::back_inserter(materialName));
		G4ParticleTable::G4PTblDicIterator* itr = G4ParticleTable::GetParticleTable()->GetIterator();
		itr->reset();
		G4bool foundIt = false;
		while ((*itr)()) {
			if (itr->value()->GetParticleName() == particleName) {
				foundIt = true;
				break;
			}
		}
		if (!foundIt) {
			std::ostringstream err;
			err << "There is no registered particle under the name " << std::quoted(particleName) << '\n';
			G4Exception("G4CherenkovProcess_Messenger::SetNewValue", "WE_ChRMessenger02", JustWarning, err);
			return;
		}
		if (!p_ChRProcess->IsApplicable(*itr->value())) {
			std::cout
				<< "A particle " << std::quoted(itr->value()->GetParticleName())
				<< " cannot generate Cherenkov photons in general!\n" << std::endl;
			return;
		}
		G4Material* theMaterial = G4Material::GetMaterial(materialName);
		if (!theMaterial) {
			std::ostringstream err;
			err << "There is no registered material under the name " << std::quoted(materialName) << '\n';
			G4Exception("G4CherenkovProcess_Messenger::SetNewValue", "WE_ChRMessenger03", JustWarning, err);
			return;
		}
		G4double minEnergy = p_ChRProcess->MinPrimaryEnergy(itr->value(), theMaterial);
		if (minEnergy > 0) {
			std::cout
			<< "The minimal energy of a particle " << std::quoted(itr->value()->GetParticleName())
			<< "\nthat is needed to generate Cherenkov photons in the registered material\n"
			<< std::quoted(theMaterial->GetName()) << " is: " << minEnergy / MeV << " MeV\n" << std::endl;
		}
		else {
			std::cout
				<< "A particle " << std::quoted(itr->value()->GetParticleName())
				<< " cannot generate Cherenkov photons\nin the registered material "
				<< std::quoted(theMaterial->GetName()) << '\n' << std::endl;
		}
	}
	else if (uiCmd == p_useEnergyLossInModels) {
		G4bool newValue = p_useEnergyLossInModels->ConvertToBool(aStr);
		for (auto* aModel : p_ChRProcess->m_registeredModels)
			aModel->SetUseModelWithEnergyLoss(newValue);
	}
	else if (uiCmd == p_noOfBetaSteps) {
		const unsigned int newBetaStep = std::stoul(aStr);
		if (newBetaStep == G4BaseChR_Model::GetNoOfBetaSteps()) {
			const char* msg = "betaStep of Cherenkov models has not been changed - you used the same number that's already set!\n";
			G4Exception("G4CherenkovProcess_Messenger::SetNewValue", "WE_ChRMessenger04", JustWarning, msg);
			return;
		}
		G4BaseChR_Model::SetNoOfBetaSteps(newBetaStep);
		if ((*p_ChRProcess->m_registeredModels.begin())->GetVerboseLevel() > 0)
			std::cout << "The betaStep value has been changed! Now deleting old physics tables...\n";
		const_cast<G4BaseChR_Model::G4ChRPhysicsTableVector&>(G4BaseChR_Model::GetChRPhysDataVec()).clear();
		if ((*p_ChRProcess->m_registeredModels.begin())->GetVerboseLevel() > 0)
			std::cout << "Old physics tables have been removed! Now creating new physics tables...\n";
		auto* particleIterator = G4ParticleTable::GetParticleTable()->GetIterator();
		particleIterator->reset();
		while ((*particleIterator)()) { //a useless and bad loop for this process, but still... pre-run time
			(*p_ChRProcess->m_registeredModels.begin())->BuildModelPhysicsTable(*(particleIterator->value()));
		}
		if((*p_ChRProcess->m_registeredModels.begin())->GetVerboseLevel() > 0)
			std::cout << "Physics tables have been successfully rebuilt!\n";
	}
	else if (uiCmd == p_modelVerboseLevel) {
		const unsigned char newValue = (const unsigned char)p_modelVerboseLevel->ConvertToInt(aStr);
		for (auto* aModel : p_ChRProcess->m_registeredModels)
			aModel->SetVerboseLevel(newValue);
	}
	else if (uiCmd == p_printPhysicsVector) {
		std::string printLevel, materialName;
		// aStr returns a space (' ') character for nothing??
		if (aStr == G4String{ ' ' }) {
			G4BaseChR_Model::PrintChRPhysDataVec();
			return;
		}
		std::string::iterator spaceChar = std::find(aStr.begin(), aStr.end(), ' ');
		std::copy(aStr.begin(), spaceChar, std::back_inserter(printLevel));
		const unsigned char printLevelNumber = (const unsigned char)std::stoul(printLevel);
		if (spaceChar + 1 != aStr.end()) {
			std::copy(spaceChar + 1, aStr.end(), std::back_inserter(materialName));
			G4Material* aMaterial = nullptr;
			if (!(aMaterial = G4Material::GetMaterial(materialName))) {
				std::ostringstream err;
				err << "You wrote that the name of a material is: " << std::quoted(materialName)
					<< "\nwhile there's no such a material. Please, check the names again!\n"
					<< "Physics tables not printed!\n";
				G4Exception("G4CherenkovProcess_Messenger::SetNewValue", "WE_ChRMessenger05", JustWarning, err);
				return;
			}
			G4BaseChR_Model::PrintChRPhysDataVec(printLevelNumber, aMaterial);
			return;
		}
		G4BaseChR_Model::PrintChRPhysDataVec(printLevelNumber);
	}
	else //just in case of some bug, but it can be removed
		G4Exception("G4CherenkovProcess_Messenger::SetNewValue", "WE_ChRMessenger06", JustWarning, "Command not found!\n");
}