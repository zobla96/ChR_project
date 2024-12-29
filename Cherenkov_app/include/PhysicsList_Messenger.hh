//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

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

class PhysicsList_Messenger final : public G4UImessenger {
public:
	PhysicsList_Messenger(PhysicsList*);
	virtual ~PhysicsList_Messenger() override;
	virtual void SetNewValue(G4UIcommand*, G4String) override;
private:
	PhysicsList* p_physicsList = nullptr;
	G4UIdirectory* p_physListDir = nullptr;
	G4UIcmdWithADoubleAndUnit* p_gammaRangeCut = nullptr;
	G4UIcmdWithADoubleAndUnit* p_electronRangeCut = nullptr;
	G4UIcmdWithADoubleAndUnit* p_positronRangeCut = nullptr;
	G4UIcmdWithADoubleAndUnit* p_protonRangeCut = nullptr;
	G4UIcmdWithAnInteger* p_selectEmPhysics = nullptr;
	G4UIcmdWithAnInteger* p_selectOpticalPhysics = nullptr;
};

endChR

#endif // !PhysicsList_Messenger_hh