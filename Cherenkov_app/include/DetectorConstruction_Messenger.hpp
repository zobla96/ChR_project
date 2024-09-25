//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef DetectorConstruction_Messenger_hpp
#define DetectorConstruction_Messenger_hpp

//User built headers
#include "DetectorConstruction.hpp"
#include "UnitsAndBench.hpp"
//G4 headers
#include "G4UImessenger.hh"
//...
#include "G4UIdirectory.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithAString.hh"
//std:: headers

beginChR

class DetectorConstruction;

class DetectorConstruction_Messenger final : public G4UImessenger {
public:
	DetectorConstruction_Messenger(DetectorConstruction*);
	~DetectorConstruction_Messenger() override;
	void SetNewValue(G4UIcommand*, G4String) override;
private:
	DetectorConstruction* p_detectorConstruction = nullptr;
	G4UIdirectory* p_detectorMessengerDir = nullptr;
	G4UIcmdWithABool* p_checkOverlap = nullptr;
	G4UIcmdWithAnInteger* p_noOfRadLayers = nullptr;
	G4UIcmdWithAString* p_radiatorMaterial = nullptr;
	G4UIcmdWithAnInteger* p_verboseLevel = nullptr;
	G4UIcmdWithADoubleAndUnit* p_radiatorAngle = nullptr;
	G4UIcmdWithADoubleAndUnit* p_radiatorThickness = nullptr;
	G4UIcmdWithADoubleAndUnit* p_detectorRadius = nullptr;
	G4UIcmdWithADoubleAndUnit* p_detectorAngle = nullptr;
	G4UIcmdWithADoubleAndUnit* p_detectorDistance = nullptr;
};

endChR

#endif // !DetectorConstruction_Messenger_hpp