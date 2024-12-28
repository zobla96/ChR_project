//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef G4ExtraOpticalParameters_Messenger_hh
#define G4ExtraOpticalParameters_Messenger_hh

//G4 headers
#include "G4UImessenger.hh"

class G4ExtraOpticalParameters;
class G4UIdirectory;
class G4UIcommand;

class G4ExtraOpticalParameters_Messenger final : public G4UImessenger {
public:
	G4ExtraOpticalParameters_Messenger(G4ExtraOpticalParameters*);
	~G4ExtraOpticalParameters_Messenger() override;
	void SetNewValue(G4UIcommand*, G4String) override;
private:
	G4ExtraOpticalParameters* p_extraOpticalParameters = nullptr;
	G4UIdirectory* p_extraOpticalParametersDIR = nullptr;
	G4UIcommand* p_materialThickness = nullptr;
	G4UIcommand* p_newScanOfLV = nullptr;
	G4UIcommand* p_executeModel = nullptr;
	G4UIcommand* p_exoticRIndex = nullptr;
	G4UIcommand* p_printChRMatData = nullptr;
};

#endif // !G4ExtraOpticalParameters_Messenger_hh