//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

// user built headers
#include "ActionInitialization.hh"
#include "RunAction.hh"
#include "TrackingAction.hh"
#include "StackingAction.hh"
#include "SteppingAction.hh"
#include "PrimaryGeneratorAction.hh"

beginChR

//=========public ChR::ActionInitialization:: methods=========

void ActionInitialization::BuildForMaster() const {
	g_runAction = new RunAction{};
	SetUserAction(g_runAction);
}

void ActionInitialization::Build() const {
	g_runAction = new RunAction{};
	SetUserAction(g_runAction);
	g_primaryGenerator = new PrimaryGeneratorAction{};
	SetUserAction(g_primaryGenerator);
	g_stackingAction = new StackingAction{};
	SetUserAction(g_stackingAction);
#ifdef boostEfficiency
	g_trackingAction = new TrackingAction{};
	SetUserAction(g_trackingAction);
#endif // boostEfficiency
#ifdef standardRun
	g_steppingAction = new SteppingAction{};
	SetUserAction(g_steppingAction);
#endif // standardRun

}

endChR