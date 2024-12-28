//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef SteppingAction_hpp
#define SteppingAction_hpp

//User built headers
#include "TrackingAction.hpp"
#include "EventAction.hpp"
#include "DetectorConstruction.hpp"
#include "PrimaryGeneratorAction.hpp"
#include "SteppingAction_Messenger.hpp"
//G4 headers
#include "G4UserSteppingAction.hh"
#include "G4Step.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4VProcess.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Tubs.hh"
//std:: headers
#include <iostream>

beginChR

class SteppingAction_Messenger;

class SteppingAction final : public G4UserSteppingAction {
public:
	SteppingAction(EventAction*, TrackingAction*, const unsigned char verbose = 0);
	~SteppingAction() override;
	void UserSteppingAction(const G4Step*) override;
	//=======Set inlines=======
	inline void SetVerboseLevel(const unsigned char);
	//=======Get inlines=======
	[[nodiscard]] inline unsigned char GetVerboseLevel() const;
private:
	TrackingAction* p_trackingAction = nullptr;
	EventAction* p_eventAction = nullptr;
	SteppingAction_Messenger* p_steppingMessenger = nullptr;
	G4RotationMatrix m_rotToDetSystem; //passive rot
	G4ThreeVector m_trToDetSurfSystem;
	const unsigned char& r_noOfRadLayers;
	const unsigned int& r_noOfPrimaries;
	unsigned char m_verboseLevel;
};

//=======Set inlines=======
void SteppingAction::SetVerboseLevel(const unsigned char val) {
	m_verboseLevel = val;
}

//=======Get inlines=======
unsigned char SteppingAction::GetVerboseLevel() const {
	return m_verboseLevel;
}

endChR

#endif // !SteppingAction_hpp