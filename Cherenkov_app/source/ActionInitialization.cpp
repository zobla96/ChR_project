//##########################################
//#######         VERSION 0.6        #######
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
	g_runAction = new RunAction{};
	SetUserAction(g_runAction);
}

void ActionInitialization::Build() const {
	g_runAction = new RunAction{};
	SetUserAction(g_runAction);
	g_eventAction = new EventAction{};
	SetUserAction(g_eventAction);
	g_primaryGenerator = new PrimaryGeneratorAction{};
	SetUserAction(g_primaryGenerator);
	g_stackingAction = new StackingAction{};
	SetUserAction(g_stackingAction);
#ifdef standardRun
	g_trackingAction = new TrackingAction{};
	SetUserAction(g_trackingAction);
	g_steppingAction = new SteppingAction{};
	SetUserAction(g_steppingAction);
#endif // standardRun

}

endChR