//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef PhysicsList_Messenger_hpp
#define PhysicsList_Messenger_hpp

//User built headers
#include "PhysicsList.hpp"
//G4 headers
#include "G4UImessenger.hh"
//...
#include "G4UIdirectory.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"

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
};

endChR

#endif // !PhysicsList_Messenger_hpp