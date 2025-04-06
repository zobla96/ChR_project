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

G4StandardCherenkovProcess UI commands
*/

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

class G4StandardChRProcess_Messenger : public G4UImessenger
{
public:
  G4StandardChRProcess_Messenger(G4StandardCherenkovProcess* theChRProcess);
  virtual ~G4StandardChRProcess_Messenger() override;
  virtual void SetNewValue(G4UIcommand* uiCmd, G4String aStr) override;
private:
  G4StandardCherenkovProcess* fStandardChRProcess;
  //DIR
  G4UIdirectory* fChRProcessDir;
  //ChR commands
  G4UIcommand* fDumpChRInfo;
  G4UIcommand* fProcessDescription;
  G4UIcmdWithAString* fIsApplicable;
  G4UIcommand* fMinEnergy;
  G4UIcmdWithAnInteger* fNoOfBetaSteps;
  G4UIcmdWithABool* fUseEnergyLoss;
  G4UIcommand* fPrintPhysicsVector;
  G4UIcommand* fExoticRIndex;
};

#endif // !G4StandardChRProcess_Messenger_hh