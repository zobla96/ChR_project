//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef ActionInitialization_hh
#define ActionInitialization_hh

// user headers
#include "DefsNConsts.hh"
// G4 headers
#include "G4VUserActionInitialization.hh"

beginChR

class ActionInitialization final : public G4VUserActionInitialization {
public:
	ActionInitialization() = default;
	~ActionInitialization() override = default;
	void BuildForMaster() const override;
	void Build() const override;
};

endChR

#endif // !ActionInitialization_hh