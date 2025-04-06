//##########################################
//#######        VERSION 1.1.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

// user headers
#include "SteppingAction_Messenger.hh"
// G4 headers
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAnInteger.hh"

beginChR

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//=========public ChR::SteppingAction_Messenger:: methods=========
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction_Messenger::SteppingAction_Messenger(SteppingAction* stepStep)
: fSteppingAction(stepStep),
  fStepActionDir(new G4UIdirectory{ "/ChR_project/SteppingAction/" }),
  fVerboseLevel(new G4UIcmdWithAnInteger{ "/ChR_project/SteppingAction/verboseLevel", this })
{

  fStepActionDir->SetGuidance("All commands related to user stepping action");

  fVerboseLevel->SetGuidance("Used to change verboseLevel of the SteppingAction class.");
  fVerboseLevel->SetParameterName("verboseLevel", true);
  fVerboseLevel->SetDefaultValue(1);
  fVerboseLevel->SetRange("verboseLevel>=0");
  fVerboseLevel->SetToBeBroadcasted(true);
  fVerboseLevel->AvailableForStates(G4State_Idle);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SteppingAction_Messenger::~SteppingAction_Messenger()
{
  delete fStepActionDir;
  delete fVerboseLevel;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SteppingAction_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr)
{
  if (uiCmd == fVerboseLevel) {
    fSteppingAction->SetVerboseLevel(fVerboseLevel->ConvertToInt(aStr));
  }
  else // just in case of some bug, but it can be removed
    G4Exception("SteppingAction_Messenger::SetNewValue", "FE_StepActionMessenger01", FatalException, "Command not found!\n");
}

endChR