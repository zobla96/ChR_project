//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef DetectorConstruction_Messenger_hh
#define DetectorConstruction_Messenger_hh

// user header
#include "DetectorConstruction.hh"
#include "DefsNConsts.hh"
// G4 header
#include "G4UImessenger.hh"

class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithADoubleAndUnit;

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
	G4UIcmdWithAString* p_radiatorMaterial = nullptr;
	G4UIcmdWithADoubleAndUnit* p_radiatorAngle = nullptr;
	G4UIcmdWithADoubleAndUnit* p_radiatorThickness = nullptr;
	G4UIcmdWithADoubleAndUnit* p_detectorRadius = nullptr;
	G4UIcmdWithADoubleAndUnit* p_detectorAngle = nullptr;
	G4UIcmdWithADoubleAndUnit* p_detectorDistance = nullptr;
};

endChR

#endif // !DetectorConstruction_Messenger_hh