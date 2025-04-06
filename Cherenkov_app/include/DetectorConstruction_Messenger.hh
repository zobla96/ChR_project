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

UI commands to modify the detector construction.
*/

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

class DetectorConstruction_Messenger final : public G4UImessenger
{
public:
  DetectorConstruction_Messenger(DetectorConstruction*);
  ~DetectorConstruction_Messenger() override;
  void SetNewValue(G4UIcommand*, G4String) override;
private:
  DetectorConstruction* fDetectorConstruction;
  G4UIdirectory* fDetectorMessengerDir;
  G4UIcmdWithAString* fRadiatorMaterial;
  G4UIcmdWithADoubleAndUnit* fRadiatorAngle;
  G4UIcmdWithADoubleAndUnit* fRadiatorThickness;
  G4UIcmdWithADoubleAndUnit* fDetectorRadius;
  G4UIcmdWithADoubleAndUnit* fDetectorAngle;
  G4UIcmdWithADoubleAndUnit* fDetectorDistance;
};

endChR

#endif // !DetectorConstruction_Messenger_hh