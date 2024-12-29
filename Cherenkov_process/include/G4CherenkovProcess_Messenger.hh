//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef G4CherenkovProcess_Messenger_hh
#define G4CherenkovProcess_Messenger_hh

//G4 headers
#include "G4UImessenger.hh"

class G4CherenkovProcess;
class G4UIDirectory;
class G4UIcommand;
class G4UIcmdWithABool;
class G4UIcmdWithAnInteger;
class G4UIcmdWithAString;

class G4CherenkovProcess_Messenger : public G4UImessenger {
public:
	G4CherenkovProcess_Messenger(G4CherenkovProcess* theChRProcess);
	virtual ~G4CherenkovProcess_Messenger() override;
	virtual void SetNewValue(G4UIcommand* uiCmd, G4String aStr) override;
private:
	G4CherenkovProcess* p_ChRProcess = nullptr;
	//2x DIR
	G4UIdirectory* p_ChRMessengerDir = nullptr;
	G4UIdirectory* p_BaseChRModelUIDirectory = nullptr;
	//ChR commands
	G4UIcommand* p_dumpChRInfo = nullptr;
	G4UIcommand* p_processDescription = nullptr;
	G4UIcmdWithAString* p_isApplicable = nullptr;
	G4UIcommand* p_minEnergy = nullptr;
	//BaseChR_Model commands
	G4UIcmdWithABool* p_useEnergyLossInModels = nullptr;
	G4UIcmdWithAnInteger* p_noOfBetaSteps = nullptr;
	G4UIcmdWithAnInteger* p_modelVerboseLevel = nullptr;
	G4UIcommand* p_printPhysicsVector = nullptr;
};

#endif // !G4CherenkovProcess_Messenger_hh