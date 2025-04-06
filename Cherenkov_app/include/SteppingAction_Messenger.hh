//##########################################
//#######        VERSION 1.1.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

/*
ABOUT THE HEADER
----------------

UI command to change the verbose level of the SteppingAction class
*/

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

class SteppingAction_Messenger final : public G4UImessenger
{
public:
  SteppingAction_Messenger(SteppingAction*);
  ~SteppingAction_Messenger() override;
  void SetNewValue(G4UIcommand*, G4String) override;
private:
  SteppingAction* fSteppingAction;
  G4UIdirectory* fStepActionDir;
  G4UIcmdWithAnInteger* fVerboseLevel;
};

endChR

#endif // !SteppingAction_Messenger_hh