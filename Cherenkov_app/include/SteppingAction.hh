//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef SteppingAction_hh
#define SteppingAction_hh

// user headers
#include "TrackingAction.hh"
#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "SteppingAction_Messenger.hh"
// G4 headers
#include "G4UserSteppingAction.hh"
// std:: headers
#include <atomic>

beginChR

class SteppingAction_Messenger;

class SteppingAction final : public G4UserSteppingAction {
public:
	SteppingAction(const G4int verbose = 0);
	~SteppingAction() override;
	void UserSteppingAction(const G4Step*) override;
	//=======Set inlines=======
	inline void SetVerboseLevel(const G4int);
	//=======Get inlines=======
	[[nodiscard]] inline G4int GetVerboseLevel() const;
	[[nodiscard]] static inline size_t GetNoOfDetections();
private:
	static std::atomic<size_t> m_noOfDetections;
	SteppingAction_Messenger* p_steppingMessenger = nullptr;
	G4RotationMatrix m_rotToDetSystem; //passive rot
	G4ThreeVector m_trToDetSurfSystem;
	G4int m_verboseLevel;
};

//=======Set inlines=======
void SteppingAction::SetVerboseLevel(const G4int val) {
	m_verboseLevel = val;
}

//=======Get inlines=======
G4int SteppingAction::GetVerboseLevel() const {
	return m_verboseLevel;
}
size_t SteppingAction::GetNoOfDetections() {
	return m_noOfDetections.load(std::memory_order_relaxed) - 1;
}

endChR

#endif // !SteppingAction_hh