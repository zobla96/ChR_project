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

ExtraOpticalParameters UI commands

Adapt the members of ExtraOpticalParameters and the way Cherenkov
processes in the G4CherenkovProcess class are executed
*/

#pragma once
#ifndef G4ExtraOpticalParameters_Messenger_hh
#define G4ExtraOpticalParameters_Messenger_hh

//G4 headers
#include "G4UImessenger.hh"

class G4ExtraOpticalParameters;
class G4UIdirectory;
class G4UIcommand;

class G4ExtraOpticalParameters_Messenger final : public G4UImessenger
{
public:
  G4ExtraOpticalParameters_Messenger(G4ExtraOpticalParameters*);
  ~G4ExtraOpticalParameters_Messenger() override;
  void SetNewValue(G4UIcommand*, G4String) override;
private:
  G4ExtraOpticalParameters* fExtraOpticalParameters;
  G4UIdirectory* fExtraOpticalParametersDIR;
  G4UIcommand* fNewScanOfLV;
  G4UIcommand* fExecuteModel;
  G4UIcommand* fExoticRIndex;
  G4UIcommand* fPrintChRMatData;
};

#endif // !G4ExtraOpticalParameters_Messenger_hh