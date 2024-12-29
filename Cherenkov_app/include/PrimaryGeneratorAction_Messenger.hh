//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef PrimaryGeneratorAction_Messenger_hh
#define PrimaryGeneratorAction_Messenger_hh

// user headers
#include "PrimaryGeneratorAction.hh"
// G4 headers
#include "G4UImessenger.hh"

class G4UIdirectory;
class G4UIcmdWithADoubleAndUnit;

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
};

endChR

#endif // !PrimaryGeneratorAction_Messenger_hh