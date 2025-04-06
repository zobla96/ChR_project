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

UI commands for the physics list
*/

#pragma once
#ifndef PhysicsList_Messenger_hh
#define PhysicsList_Messenger_hh

// user headers
#include "PhysicsList.hh"
// G4 headers
#include "G4UImessenger.hh"

class G4UIdirectory;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithAnInteger;

beginChR

class PhysicsList;

class PhysicsList_Messenger final : public G4UImessenger
{
public:
  PhysicsList_Messenger(PhysicsList*);
  virtual ~PhysicsList_Messenger() override;
  virtual void SetNewValue(G4UIcommand*, G4String) override;
private:
  PhysicsList* fPhysicsList;
  G4UIdirectory* fPhysListDir;
  G4UIcmdWithADoubleAndUnit* fGammaRangeCut;
  G4UIcmdWithADoubleAndUnit* fElectronRangeCut;
  G4UIcmdWithADoubleAndUnit* fPositronRangeCut;
  G4UIcmdWithADoubleAndUnit* fProtonRangeCut;
  G4UIcmdWithAnInteger* fSelectEmPhysics;
  G4UIcmdWithAnInteger* fSelectOpticalPhysics;
};

endChR

#endif // !PhysicsList_Messenger_hh