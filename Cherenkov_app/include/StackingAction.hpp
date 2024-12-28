//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef StackingAction_hpp
#define StackingAction_hpp

//User built headers
#include "DefsNConsts.hpp"
#include "PrimaryGeneratorAction.hpp"
//G4 headers
#include "G4UserStackingAction.hh"
#include "G4Track.hh"

beginChR

class StackingAction final : public G4UserStackingAction {
public:
	StackingAction() = default;
	~StackingAction() override = default;
	G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack) override;
};

endChR

#endif // !StackingAction_hpp