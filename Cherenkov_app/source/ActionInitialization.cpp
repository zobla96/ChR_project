//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

///User built headers
#include "ActionInitialization.hpp"
#include "RunAction.hpp"
#include "EventAction.hpp"
#include "TrackingAction.hpp"
#include "StackingAction.hpp"
#include "SteppingAction.hpp"
#include "PrimaryGeneratorAction.hpp"

beginChR

//=========public ChR::ActionInitialization:: methods=========

void ActionInitialization::BuildForMaster() const {
	SetUserAction(new RunAction{ new EventAction{} });
}

void ActionInitialization::Build() const {
	EventAction* evAction = new EventAction{};
	SetUserAction(evAction);
	SetUserAction(new RunAction{ evAction });
	SetUserAction(new StackingAction);
	SetUserAction(PrimaryGeneratorAction::GetInstance());
#ifdef standardRun
	TrackingAction* trAction = new TrackingAction{};
	SetUserAction(trAction);
	SetUserAction(new SteppingAction{ evAction, trAction });
#endif // standardRun

}

endChR