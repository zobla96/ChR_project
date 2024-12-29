//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef SteppingAction_Messenger_hh
#define SteppingAction_Messenger_hh

// user headers
#include "SteppingAction.hh"
// G4 headers
#include "G4UImessenger.hh"

class G4UIdirectory;
class G4UIcmdWithAnInteger;

beginChR

class SteppingAction;

class SteppingAction_Messenger final : public G4UImessenger {
public:
	SteppingAction_Messenger(SteppingAction*);
	~SteppingAction_Messenger() override;
	void SetNewValue(G4UIcommand*, G4String) override;
private:
	SteppingAction* p_steppingAction = nullptr;
	G4UIdirectory* p_stepActionDir = nullptr;
	G4UIcmdWithAnInteger* p_verboseLevel = nullptr;
};

endChR

#endif // !SteppingAction_Messenger_hh