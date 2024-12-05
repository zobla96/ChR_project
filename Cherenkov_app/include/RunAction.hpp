//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef RunAction_hpp
#define RunAction_hpp

//User built headers
#include "EventAction.hpp"
#include "PrimaryGeneratorAction.hpp"
#include "DetectorConstruction.hpp"
#include "PhysicsList.hpp"
#include "SteppingAction.hpp"
#include "StackingAction.hpp"
//G4 headers
#include "G4UserRunAction.hh"
//...
#include "G4RunManager.hh"
#include "G4IonTable.hh"
#include "G4AnalysisManager.hh"
#include "G4ProcessTable.hh"
//std:: headers
#include <ctime>

beginChR

class RunAction final : public G4UserRunAction {
public:
	RunAction();
	~RunAction() override = default;
	void BeginOfRunAction(const G4Run*) override;
	void EndOfRunAction(const G4Run*) override;
private:
	void LoadPrimaryGeneratorData();
};

endChR

#endif // !RunAction_hpp