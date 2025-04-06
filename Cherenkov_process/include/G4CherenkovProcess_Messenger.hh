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

Use UI commands to modify G4CherenkovProcess and registered models
*/

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

class G4CherenkovProcess_Messenger : public G4UImessenger
{
public:
  G4CherenkovProcess_Messenger(G4CherenkovProcess* theChRProcess);
  virtual ~G4CherenkovProcess_Messenger() override;
  virtual void SetNewValue(G4UIcommand* uiCmd, G4String aStr) override;
private:
  G4CherenkovProcess* fChRProcess;
  //2x DIR
  G4UIdirectory* fChRMessengerDir;
  G4UIdirectory* fBaseChRModelUIDirectory;
  //ChR commands
  G4UIcommand* fDumpChRInfo;
  G4UIcommand* fProcessDescription;
  G4UIcmdWithAString* fIsApplicable;
  G4UIcommand* fMinEnergy;
  //BaseChR_Model commands
  G4UIcmdWithABool* fUseEnergyLossInModels;
  G4UIcmdWithAnInteger* fNoOfBetaSteps;
  G4UIcmdWithAnInteger* fModelVerboseLevel;
  G4UIcommand* fPrintPhysicsVector;
};

#endif // !G4CherenkovProcess_Messenger_hh