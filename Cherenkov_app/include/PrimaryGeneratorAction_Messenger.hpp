//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef PrimaryGeneratorAction_Messenger_hpp
#define PrimaryGeneratorAction_Messenger_hpp

//User built headers
#include "PrimaryGeneratorAction.hpp"
//G4 headers
#include "G4UImessenger.hh"
//...
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
//std:: headers

beginChR

class PrimaryGeneratorAction;

class PrimaryGeneratorAction_Messenger final : public G4UImessenger {
public:
	PrimaryGeneratorAction_Messenger(PrimaryGeneratorAction*);
	~PrimaryGeneratorAction_Messenger() override;
	void SetNewValue(G4UIcommand*, G4String) override;
private:
	PrimaryGeneratorAction* p_primaryGenerator = nullptr;
	G4UIdirectory* p_pGeneratorMessengerDir = nullptr;
	G4UIcmdWithADoubleAndUnit* p_beamSigma = nullptr;
	G4UIcmdWithADoubleAndUnit* p_divSigma = nullptr;
	G4UIcmdWithADoubleAndUnit* p_energy = nullptr;
	G4UIcmdWithAnInteger* p_noOfParticles = nullptr;
	G4UIcmdWithAnInteger* p_massNo = nullptr;
	G4UIcmdWithAnInteger* p_atomicNo = nullptr;
};

endChR

#endif // !PrimaryGeneratorAction_Messenger_hpp