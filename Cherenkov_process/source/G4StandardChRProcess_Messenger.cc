//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#include "G4StandardChRProcess_Messenger.hh"
#include "G4StandardCherenkovProcess.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4MTRunManager.hh"

//=========public G4StandardChRProcess_Messenger:: methods=========

G4StandardChRProcess_Messenger::G4StandardChRProcess_Messenger(G4StandardCherenkovProcess* theChRProcess)
: p_standardChRProcess(theChRProcess) {
	//DIR
	p_ChRProcessDir = new G4UIdirectory{ "/process/optical/stdChRProcess/" };
	p_ChRProcessDir->SetGuidance("All commands related to G4StandardCherenkovProcess");
	//commands
	p_dumpChRInfo = new G4UIcommand{ "/process/optical/stdChRProcess/dumpInfo", this };
	p_dumpChRInfo->SetGuidance("Used to print information about the G4StandardCherenkovProcess class.");
	p_dumpChRInfo->SetToBeBroadcasted(false);
	p_dumpChRInfo->AvailableForStates(G4State_Idle, G4State_GeomClosed, G4State_EventProc);

	p_processDescription = new G4UIcommand{ "/process/optical/stdChRProcess/processDescription", this };
	p_processDescription->SetGuidance("Used to print about what Cherenkov process is.");
	p_processDescription->SetToBeBroadcasted(false);
	p_processDescription->AvailableForStates(G4State_Idle, G4State_GeomClosed, G4State_EventProc);

	p_isApplicable = new G4UIcmdWithAString{ "/process/optical/stdChRProcess/isApplicable", this };
	p_isApplicable->SetGuidance("Check if a specific particle can generate Cherenkov photons.");
	p_isApplicable->SetGuidance("NOTE: Make sure to write a particle name correctly.");
	p_isApplicable->SetParameterName("particleName", false);
	p_isApplicable->SetToBeBroadcasted(false);
	p_isApplicable->AvailableForStates(G4State_Idle, G4State_GeomClosed, G4State_EventProc);

	p_minEnergy = new G4UIcommand{ "/process/optical/stdChRProcess/minEnergy", this };
	p_minEnergy->SetGuidance("Check if a specific particle can generate Cherenkov photons");
	p_minEnergy->SetGuidance("for a specific material, and print the minimal energy");
	p_minEnergy->SetGuidance("needed for Cherenkov photons' generation.");
	p_minEnergy->SetGuidance("You need to provide two strings:");
	p_minEnergy->SetGuidance("1. A name of the particle");
	p_minEnergy->SetGuidance("2. A name of the registered material");
	//G4UIParameter objects are deleted in deconstructor of G4UIcommand
	G4UIparameter* uiParameter = new G4UIparameter{ "particleName", 's', false };
	p_minEnergy->SetParameter(uiParameter);
	uiParameter = new G4UIparameter{ "materialName", 's', false };
	p_minEnergy->SetParameter(uiParameter);
	p_minEnergy->SetToBeBroadcasted(false);
	p_minEnergy->AvailableForStates(G4State_Idle, G4State_GeomClosed, G4State_EventProc);

	p_useEnergyLoss = new G4UIcmdWithABool{ "/process/optical/stdChRProcess/useEnergyLoss", this };
	p_useEnergyLoss->SetGuidance("Used to activate energy conservation law for Cherenkov process.");
	p_useEnergyLoss->SetGuidance("According to G4Cerenkov and Frank-Tamm theory, Cherenkov radiation doesn't change charged particle's energy.");
	p_useEnergyLoss->SetGuidance("However, if you select \"true\" in new models, the energy change is considered (note that it's minimal anyway).");
	p_useEnergyLoss->SetParameterName("changeEnergyInChR", true);
	p_useEnergyLoss->SetDefaultValue(true);
	p_useEnergyLoss->SetToBeBroadcasted(true);
	p_useEnergyLoss->AvailableForStates(G4State_Idle);

	p_noOfBetaSteps = new G4UIcmdWithAnInteger{ "/process/optical/stdChRProcess/noOfBetaSteps", this };
	p_noOfBetaSteps->SetGuidance("Used to change the number of beta steps (betaNodes == betaStep + 1).");
	p_noOfBetaSteps->SetGuidance("When building physics tables for G4StandardChRProcess, the critical energies are considered through the relativistic velocity \"beta\" of the charged particle.");
	p_noOfBetaSteps->SetGuidance("For critical energies, the physics tables are divided into steps from minimal beta to maximal beta.");
	p_noOfBetaSteps->SetParameterName("betaSteps", false);
	p_noOfBetaSteps->SetRange("betaSteps>=1 && betaSteps<=255");
	p_noOfBetaSteps->SetToBeBroadcasted(false);
	p_noOfBetaSteps->AvailableForStates(G4State_Idle);

	p_printPhysicsVector = new G4UIcommand{ "/process/optical/stdChRProcess/printChRPhysicsVector", this };
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
	p_printPhysicsVector->AvailableForStates(G4State_Idle, G4State_GeomClosed, G4State_EventProc);

	p_exoticRIndex = new G4UIcommand{ "/process/optical/stdChRProcess/changeUseOfExoticRIndex", this };
	p_exoticRIndex->SetGuidance("Used to change whether exotic refractive indices will be used for a");
	p_exoticRIndex->SetGuidance("specific material or not.");
	p_exoticRIndex->SetGuidance("IF YOU CHANGE THE NUMBER OF BETA STEPS BY COMMAND:");
	p_exoticRIndex->SetGuidance("\"/process/optical/stdChRProcess/noOfBetaSteps\"");
	p_exoticRIndex->SetGuidance("AFTER USING THIS COMMAND, YOU WILL LOSE THE RESULT OF THIS COMMAND!");
	uiParameter = new G4UIparameter{ "materialName", 's', false };
	p_exoticRIndex->SetParameter(uiParameter);
	uiParameter = new G4UIparameter{ "exoticRIndex", 'b', false };
	p_exoticRIndex->SetParameter(uiParameter);
	p_exoticRIndex->SetToBeBroadcasted(false);
	p_exoticRIndex->AvailableForStates(G4State_Idle);
}

G4StandardChRProcess_Messenger::~G4StandardChRProcess_Messenger() {
	//DIR
	delete p_ChRProcessDir;
	//ChR commands
	delete p_dumpChRInfo;
	delete p_processDescription;
	delete p_isApplicable;
	delete p_minEnergy;
	delete p_noOfBetaSteps;
	delete p_useEnergyLoss;
	delete p_printPhysicsVector;
	delete p_exoticRIndex;
}

void G4StandardChRProcess_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr) {
	if (uiCmd == p_dumpChRInfo) {
		p_standardChRProcess->DumpInfo();
	}
	else if (uiCmd == p_processDescription) {
		p_standardChRProcess->ProcessDescription();
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
			if (p_standardChRProcess->IsApplicable(*(itr->value())))
				std::cout << "Particle " << aStr << " can generate Cherenkov photons!\n";
			else
				std::cout << "Particle " << aStr << " cannot generate Cherenkov photons!\n";
		}
		else {
			std::ostringstream err;
			err << "There is no registered particle under the name " << std::quoted(aStr) << '\n';
			G4Exception("G4StandardChRProcess_Messenger::SetNewValue", "WE_stdChRMessenger01", JustWarning, err);
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
			G4Exception("G4StandardChRProcess_Messenger::SetNewValue", "WE_stdChRMessenger02", JustWarning, err);
			return;
		}
		if (!p_standardChRProcess->IsApplicable(*itr->value())) {
			std::cout
				<< "A particle " << std::quoted(itr->value()->GetParticleName())
				<< " cannot generate Cherenkov photons in general!\n" << std::endl;
			return;
		}
		G4Material* theMaterial = G4Material::GetMaterial(materialName);
		if (!theMaterial) {
			std::ostringstream err;
			err << "There is no registered material under the name " << std::quoted(materialName) << '\n';
			G4Exception("G4StandardChRProcess_Messenger::SetNewValue", "WE_stdChRMessenger03", JustWarning, err);
			return;
		}
		G4double minEnergy = p_standardChRProcess->MinPrimaryEnergy(itr->value(), theMaterial);
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
	else if (uiCmd == p_noOfBetaSteps) {
		const unsigned int newBetaStep = std::stoul(aStr);
		if (newBetaStep == p_standardChRProcess->GetNoOfBetaSteps()) {
			const char* msg = "betaStep of Cherenkov models has not been changed - you used the same number that's already set!\n";
			G4Exception("G4StandardChRProcess_Messenger::SetNewValue", "WE_stdChRMessenger04", JustWarning, msg);
			return;
		}
		p_standardChRProcess->SetNoOfBetaSteps(newBetaStep);
		if (p_standardChRProcess->verboseLevel > 0)
			std::cout << "All betaStep values have been changed! Now deleting old physics tables...\n";
		p_standardChRProcess->m_ChRPhysDataVec.clear();
		if (p_standardChRProcess->verboseLevel > 0)
			std::cout << "Old physics tables have been removed! Now creating new physics tables...\n";
		auto* particleIterator = G4ParticleTable::GetParticleTable()->GetIterator();
		particleIterator->reset();
		while ((*particleIterator)()) { //a useless and bad loop for this process, but still... pre-run time
			p_standardChRProcess->BuildPhysicsTable(*(particleIterator->value()));
		}
		if (p_standardChRProcess->verboseLevel > 0)
			std::cout << "Physics tables have been successfully rebuilt!\n";
	}
	else if (uiCmd == p_useEnergyLoss) {
		p_standardChRProcess->SetUseEnergyLoss(p_useEnergyLoss->ConvertToBool(aStr));
	}
	else if (uiCmd == p_printPhysicsVector) {
		std::string printLevel, materialName;
		// aStr returns a space (' ') character for nothing??
		if (aStr == G4String{ ' ' }) {
			p_standardChRProcess->PrintChRPhysDataVec();
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
				G4Exception("G4StandardChRProcess_Messenger::SetNewValue", "WE_stdChRMessenger05", JustWarning, err);
				return;
			}
			p_standardChRProcess->PrintChRPhysDataVec(printLevelNumber, aMaterial);
			return;
		}
		p_standardChRProcess->PrintChRPhysDataVec(printLevelNumber);
	}
	else if (uiCmd == p_exoticRIndex) {
		std::string matName, newFlag;
		std::string::iterator spaceChar1 = std::find(aStr.begin(), aStr.end(), ' ');
		std::copy(aStr.begin(), spaceChar1, std::back_inserter(matName));
		std::string::iterator spaceChar2 = std::find(spaceChar1 + 1, aStr.end(), ' ');
		std::copy(spaceChar1 + 1, spaceChar2, std::back_inserter(newFlag));
		G4Material* aMaterial = G4Material::GetMaterial(matName);
		if (!aMaterial) {
			std::ostringstream err;
			err << "You wrote that the name of a material is: " << std::quoted(matName)
				<< "\nwhile there's no such a material. Please, check the names again!\n";
			G4Exception("G4ExtraOpticalParameters_Messenger::SetNewValue", "WE_stdChRMessenger06", JustWarning, err);
			return;
		}
		G4bool tableExists;
		if (p_standardChRProcess->m_ChRPhysDataVec[aMaterial->GetIndex()].m_aroundBetaValues.front().p_valuesCDF)
			tableExists = true;
		else
			tableExists = false;
		G4bool newValue = p_exoticRIndex->ConvertToBool(newFlag.c_str());
		if (newValue == tableExists)
			return;
		if (newValue) {
			if (!p_standardChRProcess->AddExoticRIndexPhysicsTable(aMaterial->GetIndex(), true)) {
				std::ostringstream err;
				err << "Exotic RIndex tables are not built for material " << std::quoted(matName)
					<< "\nThe material's RIndex is not suitable for exotic RIndex tables!\n";
				G4Exception("G4ExtraOpticalParameters_Messenger::SetNewValue", "WE_stdChRMessenger07", JustWarning, err);
			}
		}
		else
			p_standardChRProcess->RemoveExoticRIndexPhysicsTable(aMaterial->GetIndex());
	}
	else { //just in case of some bug, but it can be removed
		G4Exception("G4StandardChRProcess_Messenger::SetNewValue", "WE_stdChRMessenger08", JustWarning, "Command not found!\n");
	}
}