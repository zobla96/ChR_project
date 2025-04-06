//##########################################
//#######        VERSION 1.1.0       #######
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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//=========public G4ExtraOpticalParameters_Messenger:: methods=========
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ExtraOpticalParameters_Messenger::G4ExtraOpticalParameters_Messenger(G4ExtraOpticalParameters* theExtraOptParam)
: fExtraOpticalParameters(theExtraOptParam),
  fExtraOpticalParametersDIR(new G4UIdirectory{ "/process/optical/G4ChRProcess/extraOptParams/" }),
  fNewScanOfLV(new G4UIcommand{ "/process/optical/G4ChRProcess/extraOptParams/scanForNewLV", this }),
  fExecuteModel(new G4UIcommand{ "/process/optical/G4ChRProcess/extraOptParams/ChRexecuteModelID", this }),
  fExoticRIndex(new G4UIcommand{ "/process/optical/G4ChRProcess/extraOptParams/exoticRIndex", this }),
  fPrintChRMatData(new G4UIcommand{ "/process/optical/G4ChRProcess/extraOptParams/printChRMatData", this })
{
  //DIR
  fExtraOpticalParametersDIR->SetGuidance("All commands related to G4ExtraOpticalParameters");

  //G4UIparameter to be used (it's deleted by G4UIcommand):
  G4UIparameter* uiParameter = nullptr;

  //ExtraOpticalParameters commands
  fNewScanOfLV->SetGuidance("Used to rescan logical volumes. You might need to use this command");
  fNewScanOfLV->SetGuidance("if you have changed the geometry in G4State_Idle and want to use G4CherenkovProcess");
  fNewScanOfLV->SetToBeBroadcasted(false);
  fNewScanOfLV->AvailableForStates(G4State_Idle);

  fExecuteModel->SetGuidance("You should use this command if you need to change a Cherenkov model that should be executed for a specific logical volume.");
  fExecuteModel->SetGuidance("You should specify the name of a logical volume and an integer of a register model.");
  uiParameter = new G4UIparameter{ "LV_name", 's', false };
  fExecuteModel->SetParameter(uiParameter);
  uiParameter = new G4UIparameter{ "modelID", 'i', false };
  uiParameter->SetParameterRange("modelID>=0");
  fExecuteModel->SetParameter(uiParameter);
  fExecuteModel->SetToBeBroadcasted(false);
  fExecuteModel->AvailableForStates(G4State_Idle);

  fExoticRIndex->SetGuidance("You should use this command to turn on/off use of an exotic refractive index for a specific logical volume.");
  fExoticRIndex->SetGuidance("Exotic refractive indices are more processor heavy, so they should not be used when not necessary.");
  fExoticRIndex->SetGuidance("Cherenkov radiation is emitted in standard E = const, if this value is set to 'false'.");
  uiParameter = new G4UIparameter{ "LV_name", 's', false };
  fExoticRIndex->SetParameter(uiParameter);
  uiParameter = new G4UIparameter{ "exoticRIndex", 'b', false };
  fExoticRIndex->SetParameter(uiParameter);
  fExoticRIndex->SetToBeBroadcasted(false);
  fExoticRIndex->AvailableForStates(G4State_Idle);

  fPrintChRMatData->SetGuidance("Use this command to print all data about Cherenkov material data, or");
  fPrintChRMatData->SetGuidance("specify a logical volume for which to print the data.");
  uiParameter = new G4UIparameter{ "LV_name", 's', true };
  fPrintChRMatData->SetParameter(uiParameter);
  fPrintChRMatData->SetToBeBroadcasted(false);
  fPrintChRMatData->AvailableForStates(G4State_Idle);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ExtraOpticalParameters_Messenger::~G4ExtraOpticalParameters_Messenger()
{
  delete fExtraOpticalParametersDIR;
  delete fNewScanOfLV;
  delete fExecuteModel;
  delete fExoticRIndex;
  delete fPrintChRMatData;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void G4ExtraOpticalParameters_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr)
{
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
  if (uiCmd == fNewScanOfLV) {
    fExtraOpticalParameters->ScanAndAddUnregisteredLV();
  }
  else if (uiCmd == fExecuteModel) {
    findLV();
    G4LogicalVolume* aLogicVolume = G4LogicalVolumeStore::GetInstance()->GetVolume(name1);
    if (!aLogicVolume) {
      std::ostringstream err;
      err << "You wrote that the name of a logical volume is: " << std::quoted(name1)
        << "\nwhile there's no such a registered volume. Please, check the names again!\n";
      G4Exception("G4ExtraOpticalParameters_Messenger::SetNewValue", "WE_EOPMessenger02", JustWarning, err);
      return;
    }
    G4CherenkovMatData& lvMatData = fExtraOpticalParameters->FindOrCreateChRMatData(aLogicVolume);
    lvMatData.m_executeModel = std::stoull(name2.c_str());
  }
  else if (uiCmd == fExoticRIndex) {
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
    G4CherenkovMatData& lvMatData = fExtraOpticalParameters->FindOrCreateChRMatData(aLogicVolume);
    G4bool newValue = fExoticRIndex->ConvertToBool(name2.c_str());
    if (newValue == lvMatData.fExoticRIndex)
      return;
    lvMatData.fExoticRIndex = newValue;
    // now, find if there's another LV with the same material. If there is, check its fExoticRIndex flag.
    // If exotic RIndex is not used anymore, remove unnecessary physics-table data and free up some memory
    // If it is in use, do nothing
    G4Material* aMaterial = aLogicVolume->GetMaterial();
    for (const auto* i : *lvStore) {
      if (i == aLogicVolume)
        continue;
      // comparing material memory addresses
      if (i->GetMaterial() == aMaterial && fExtraOpticalParameters->FindOrCreateChRMatData(i).fExoticRIndex == true)
        return; // the other LV with the given material keeps physics table, no matter what
    }
    if (newValue) {
      if (!G4BaseChR_Model::AddExoticRIndexPhysicsTable(aMaterial->GetIndex(), true)) {
        lvMatData.fExoticRIndex = false;
        const char* err = "fExoticRIndex flag did not successfully change to true!\nThe material's RIndex is not suitable for the 'true' fExoticRIndex flag condition!\n";
        G4Exception("G4ExtraOpticalParameters_Messenger::SetNewValue", "WE_EOPMessenger04", JustWarning, err);
      }
    }
    else
      G4BaseChR_Model::RemoveExoticRIndexPhysicsTable(aMaterial->GetIndex());
  }
  else if (uiCmd == fPrintChRMatData) {
    if (!aStr.empty()) {
      const G4LogicalVolume* aLogicVolume = G4LogicalVolumeStore::GetInstance()->GetVolume(aStr);
      if (!aLogicVolume) {
        std::ostringstream err;
        err << "You wrote that the name of a logical volume is: " << std::quoted(aStr)
          << "\nwhile there's no such a registered volume. Please, check the names again!\n";
        G4Exception("G4ExtraOpticalParameters_Messenger::SetNewValue", "WE_EOPMessenger05", JustWarning, err);
        return;
      }
      fExtraOpticalParameters->PrintChRMatData(aLogicVolume);
    }
    fExtraOpticalParameters->PrintChRMatData();
  }
  else { //just in case of some bug, but it can be removed
    G4Exception("G4ExtraOpticalParameters_Messenger::SetNewValue", "WE_EOPMessenger06", JustWarning, "Command not found!\n");
  }
}