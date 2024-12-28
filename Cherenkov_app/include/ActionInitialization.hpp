//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef ActionInitialization_hpp
#define ActionInitialization_hpp

//User built headers
#include "UnitsAndBench.hpp"
//G4 headers
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

#endif // !ActionInitialization_hpp