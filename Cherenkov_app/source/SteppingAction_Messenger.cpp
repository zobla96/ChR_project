//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#include "SteppingAction_Messenger.hpp"

beginChR

//=========public ChR::SteppingAction_Messenger:: methods=========

SteppingAction_Messenger::SteppingAction_Messenger(SteppingAction* stepStep)
: p_steppingAction(stepStep) {
	p_stepActionDir = new G4UIdirectory{ "/ChR_project/SteppingAction/" };
	p_stepActionDir->SetGuidance("All commands related to user stepping action");

	p_verboseLevel = new G4UIcmdWithAnInteger{ "/ChR_project/SteppingAction/verboseLevel", this };
	p_verboseLevel->SetGuidance("Used to change verboseLevel of the SteppingAction class.");
	p_verboseLevel->SetParameterName("verboseLevel", true);
	p_verboseLevel->SetDefaultValue(1);
	p_verboseLevel->SetRange("verboseLevel>=0 && verboseLevel<256");
	p_verboseLevel->SetToBeBroadcasted(true);
	p_verboseLevel->AvailableForStates(G4State_Idle);
}

SteppingAction_Messenger::~SteppingAction_Messenger() {
	delete p_stepActionDir;
	delete p_verboseLevel;
}

void SteppingAction_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr) {
	if (uiCmd == p_verboseLevel) {
		p_steppingAction->SetVerboseLevel((unsigned char)p_verboseLevel->ConvertToInt(aStr));
	}
	else //just in case of some bug, but it can be removed
		G4Exception("SteppingAction_Messenger::SetNewValue", "WE_StepActionMessenger01", JustWarning, "Command not found!\n");
}

endChR