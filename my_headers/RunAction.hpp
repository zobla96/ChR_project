#pragma once
#ifndef RunAction_hpp
#define RunAction_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "EventAction.hpp"
#include "PrimaryGeneratorAction.hpp"
#include "DetectorConstruction.hpp"
#include "ProcessCsvData.hpp"
#include "PhysicsList.hpp"
#include "SteppingAction.hpp"
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
	RunAction(const EventAction*);
	~RunAction() override;
	void BeginOfRunAction(const G4Run*) override;
	void EndOfRunAction(const G4Run*) override;
private:
	void LoadPrimaryGeneratorData();
	const EventAction* p_eventAction;
	std::string m_fileName;
	std::string m_fileExtension;
	std::string m_ntName;
};

endChR

#endif // !RunAction_hpp