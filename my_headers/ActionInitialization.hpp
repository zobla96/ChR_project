#pragma once
#ifndef ActionInitialization_hpp
#define ActionInitialization_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "RunAction.hpp"
#include "EventAction.hpp"
#include "TrackingAction.hpp"
#include "StackingAction.hpp"
#include "SteppingAction.hpp"
#include "PrimaryGeneratorAction.hpp"
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