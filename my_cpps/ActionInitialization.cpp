#include "ActionInitialization.hpp"

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

beginChR

//=========public ChR::ActionInitialization:: methods=========
void ActionInitialization::BuildForMaster() const {
	SetUserAction(new RunAction{ new EventAction{} });
}

void ActionInitialization::Build() const {
	EventAction* evAction = new EventAction{};
	SetUserAction(new RunAction{ evAction });
	SetUserAction(evAction);
	TrackingAction* trAction = new TrackingAction{};
	SetUserAction(trAction);
	SetUserAction(new StackingAction);
	SetUserAction(new SteppingAction{ evAction, trAction, 2 });
	SetUserAction(PrimaryGeneratorAction::GetInstance());
}

endChR