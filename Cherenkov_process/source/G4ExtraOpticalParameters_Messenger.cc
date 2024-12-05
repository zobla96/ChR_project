//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//G4 headers
#include "G4ExtraOpticalParameters_Messenger.hh"
#include "G4ExtraOpticalParameters.hh"
#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIparameter.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4BaseChR_Model.hh"
#include "G4Material.hh"
#include "G4CherenkovProcess.hh"
#include "G4ProcessTable.hh"
//std:: headers
#include <functional>

//=========public G4ExtraOpticalParameters_Messenger:: methods=========

G4ExtraOpticalParameters_Messenger::G4ExtraOpticalParameters_Messenger(G4ExtraOpticalParameters* theExtraOptParam)
: p_extraOpticalParameters(theExtraOptParam) {
	//DIR
	p_extraOpticalParametersDIR = new G4UIdirectory{ "/process/optical/G4ChRProcess/extraOptParams/" };
	p_extraOpticalParametersDIR->SetGuidance("All commands related to G4ExtraOpticalParameters");
	
	//G4UIparameter to be used (it's deleted by G4UIcommand):
	G4UIparameter* uiParameter = nullptr;
	
	//ExtraOpticalParameters commands
	p_newScanOfLV = new G4UIcommand{ "/process/optical/G4ChRProcess/extraOptParams/scanForNewLV", this };
	p_newScanOfLV->SetGuidance("Used to rescan logical volumes. You might need to use this command");
	p_newScanOfLV->SetGuidance("if you have changed the geometry in G4State_Idle and want to use G4CherenkovProcess");
	p_newScanOfLV->SetToBeBroadcasted(false);
	p_newScanOfLV->AvailableForStates(G4State_Idle);

	p_executeModel = new G4UIcommand{ "/process/optical/G4ChRProcess/extraOptParams/ChRexecuteModelID", this };
	p_executeModel->SetGuidance("You should use this command if you need to change a Cherenkov model that should be executed for a specific logical volume.");
	p_executeModel->SetGuidance("You should specify the name of a logical volume and an integer of a register model.");
	uiParameter = new G4UIparameter{ "LV_name", 's', false };
	p_executeModel->SetParameter(uiParameter);
	uiParameter = new G4UIparameter{ "modelID", 'i', false };
	uiParameter->SetParameterRange("modelID>=0");
	p_executeModel->SetParameter(uiParameter);
	p_executeModel->SetToBeBroadcasted(false);
	p_executeModel->AvailableForStates(G4State_Idle);

	p_exoticRIndex = new G4UIcommand{ "/process/optical/G4ChRProcess/extraOptParams/exoticRIndex", this };
	p_exoticRIndex->SetGuidance("You should use this command to turn on/off use of an exotic refractive index for a specific logical volume.");
	p_exoticRIndex->SetGuidance("Exotic refractive indices are more processor heavy, so they should not be used when not necessary.");
	p_exoticRIndex->SetGuidance("Cherenkov radiation is emitted in standard E = const, if this value is set to 'false'.");
	uiParameter = new G4UIparameter{ "LV_name", 's', false };
	p_exoticRIndex->SetParameter(uiParameter);
	uiParameter = new G4UIparameter{ "exoticRIndex", 'b', false};
	p_exoticRIndex->SetParameter(uiParameter);
	p_exoticRIndex->SetToBeBroadcasted(false);
	p_exoticRIndex->AvailableForStates(G4State_Idle);

	p_printChRMatData = new G4UIcommand{ "/process/optical/G4ChRProcess/extraOptParams/printChRMatData", this };
	p_printChRMatData->SetGuidance("Use this command to print all data about Cherenkov material data, or");
	p_printChRMatData->SetGuidance("specify a logical volume for which to print the data.");
	uiParameter = new G4UIparameter{ "LV_name", 's', true };
	p_printChRMatData->SetParameter(uiParameter);
	p_printChRMatData->SetToBeBroadcasted(false);
	p_printChRMatData->AvailableForStates(G4State_Idle);
}

G4ExtraOpticalParameters_Messenger::~G4ExtraOpticalParameters_Messenger() {
	delete p_extraOpticalParametersDIR;
	delete p_newScanOfLV;
	delete p_executeModel;
	delete p_exoticRIndex;
	delete p_printChRMatData;
}

void G4ExtraOpticalParameters_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr) {
	std::string name1, name2, name3;
	std::function<void()> findLV = [&] {
		std::string::iterator spaceChar1 = std::find(aStr.begin(), aStr.end(), ' ');
		std::copy(aStr.begin(), spaceChar1, std::back_inserter(name1));
		std::string::iterator spaceChar2 = std::find(spaceChar1 + 1, aStr.end(), ' ');
		std::copy(spaceChar1 + 1, spaceChar2, std::back_inserter(name2));
		if (spaceChar2 != aStr.end())
			std::copy(spaceChar2 + 1, aStr.end(), std::back_inserter(name3)); //3 strings
		//else 2 strings
		/*
		I haven't noticed anything in the source code of the UI classes, but there might be a better
		way to do the previous (like an inbuilt function of G4).
		*/
	};
	if (uiCmd == p_newScanOfLV) {
		p_extraOpticalParameters->ScanAndAddUnregisteredLV();
	}
	else if (uiCmd == p_executeModel) {
		findLV();
		G4LogicalVolume* aLogicVolume = G4LogicalVolumeStore::GetInstance()->GetVolume(name1);
		if (!aLogicVolume) {
			std::ostringstream err;
			err << "You wrote that the name of a logical volume is: " << std::quoted(name1)
				<< "\nwhile there's no such a registered volume. Please, check the names again!\n";
			G4Exception("G4ExtraOpticalParameters_Messenger::SetNewValue", "WE_EOPMessenger02", JustWarning, err);
			return;
		}
		G4CherenkovMatData& lvMatData = p_extraOpticalParameters->FindOrCreateChRMatData(aLogicVolume);
		lvMatData.m_executeModel = std::stoull(name2.c_str());
	}
	else if (uiCmd == p_exoticRIndex) {
		findLV();
		G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
		G4LogicalVolume* aLogicVolume = lvStore->GetVolume(name1);
		if (!aLogicVolume) {
			std::ostringstream err;
			err << "You wrote that the name of a logical volume is: " << std::quoted(name1)
				<< "\nwhile there's no such a registered volume. Please, check the names again!\n";
			G4Exception("G4ExtraOpticalParameters_Messenger::SetNewValue", "WE_EOPMessenger03", JustWarning, err);
			return;
		}
		G4CherenkovMatData& lvMatData = p_extraOpticalParameters->FindOrCreateChRMatData(aLogicVolume);
		G4bool newValue = p_exoticRIndex->ConvertToBool(name2.c_str());
		if (newValue == lvMatData.m_exoticRIndex)
			return;
		lvMatData.m_exoticRIndex = newValue;
		// now, find if there's another LV with the same material. If there is, check its m_exoticRIndex flag.
		// If exotic RIndex is not used anymore, remove unnecessary physics-table data and free up some memory
		// If it is in use, do nothing
		G4Material* aMaterial = aLogicVolume->GetMaterial();
		for (const auto* i : *lvStore) {
			if (i == aLogicVolume)
				continue;
			// comparing material memory addresses
			if (i->GetMaterial() == aMaterial && p_extraOpticalParameters->FindOrCreateChRMatData(i).m_exoticRIndex == true)
				return; // the other LV with the given material keeps physics table, no matter what
		}
		if (newValue) {
			if (!G4BaseChR_Model::AddExoticRIndexPhysicsTable(aMaterial->GetIndex(), true)) {
				lvMatData.m_exoticRIndex = false;
				const char* err = "m_exoticRIndex flag did not successfully change to true!\nThe material's RIndex is not suitable for the 'true' m_exoticRIndex flag condition!\n";
				G4Exception("G4ExtraOpticalParameters_Messenger::SetNewValue", "WE_EOPMessenger04", JustWarning, err);
			}
		}
		else
			G4BaseChR_Model::RemoveExoticRIndexPhysicsTable(aMaterial->GetIndex());
	}
	else if (uiCmd == p_printChRMatData) {
		if (!aStr.empty()) {
			const G4LogicalVolume* aLogicVolume = G4LogicalVolumeStore::GetInstance()->GetVolume(aStr);
			if (!aLogicVolume) {
				std::ostringstream err;
				err << "You wrote that the name of a logical volume is: " << std::quoted(aStr)
					<< "\nwhile there's no such a registered volume. Please, check the names again!\n";
				G4Exception("G4ExtraOpticalParameters_Messenger::SetNewValue", "WE_EOPMessenger05", JustWarning, err);
				return;
			}
			p_extraOpticalParameters->PrintChRMatData(aLogicVolume);
		}
		p_extraOpticalParameters->PrintChRMatData();
	}
	else { //just in case of some bug, but it can be removed
		G4Exception("G4ExtraOpticalParameters_Messenger::SetNewValue", "WE_EOPMessenger06", JustWarning, "Command not found!\n");
	}
}