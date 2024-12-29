//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef G4StandardChRProcess_Messenger_hh
#define G4StandardChRProcess_Messenger_hh

//G4 headers
#include "G4UImessenger.hh"

class G4StandardCherenkovProcess;
class G4UIdirectory;
class G4UIcommand;
class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;
class G4UIcmdWithABool;

class G4StandardChRProcess_Messenger : public G4UImessenger {
public:
	G4StandardChRProcess_Messenger(G4StandardCherenkovProcess* theChRProcess);
	virtual ~G4StandardChRProcess_Messenger() override;
	virtual void SetNewValue(G4UIcommand* uiCmd, G4String aStr) override;
private:
	G4StandardCherenkovProcess* p_standardChRProcess = nullptr;
	//DIR
	G4UIdirectory* p_ChRProcessDir = nullptr;
	//ChR commands
	G4UIcommand* p_dumpChRInfo = nullptr;
	G4UIcommand* p_processDescription = nullptr;
	G4UIcmdWithAString* p_isApplicable = nullptr;
	G4UIcommand* p_minEnergy = nullptr;
	G4UIcmdWithAnInteger* p_noOfBetaSteps = nullptr;
	G4UIcmdWithABool* p_useEnergyLoss = nullptr;
	G4UIcommand* p_printPhysicsVector = nullptr;
	G4UIcommand* p_exoticRIndex = nullptr;
};

#endif // !G4StandardChRProcess_Messenger_hh