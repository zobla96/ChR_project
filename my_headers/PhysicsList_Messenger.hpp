#pragma once
#ifndef PhysicsList_Messenger_hpp
#define PhysicsList_Messenger_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "PhysicsList.hpp"
//G4 headers
#include "G4UImessenger.hh"
//...
#include "G4UIdirectory.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"
//std:: headers

beginChR

class PhysicsList;

class PhysicsList_Messenger : public G4UImessenger {
public:
	explicit PhysicsList_Messenger(PhysicsList*);
	virtual ~PhysicsList_Messenger() override;
	virtual void SetNewValue(G4UIcommand*, G4String) override;
private:
	PhysicsList* p_physicsList = nullptr;
	G4UIdirectory* p_physListDir = nullptr;
	G4UIcmdWithABool* p_useNonDefaultCuts = nullptr;
	G4UIcmdWithADoubleAndUnit* p_gammaRangeCut = nullptr;
	G4UIcmdWithADoubleAndUnit* p_electronRangeCut = nullptr;
	G4UIcmdWithADoubleAndUnit* p_positronRangeCut = nullptr;
	G4UIcmdWithADoubleAndUnit* p_protonRangeCut = nullptr;
	G4UIcmdWithAnInteger* p_selectEmPhysics = nullptr;
};

endChR

#endif // !PhysicsList_Messenger_hpp