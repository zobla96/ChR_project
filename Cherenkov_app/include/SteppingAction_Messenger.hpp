//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef SteppingAction_Messenger_hpp
#define SteppingAction_Messenger_hpp

//User built headers
#include "SteppingAction.hpp"
//G4 headers
#include "G4UImessenger.hh"
//...
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAnInteger.hh"

beginChR

class SteppingAction;

//might change this one to "VerboseOnlyMessenger" template class in the future

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

#endif // !SteppingAction_Messenger_hpp