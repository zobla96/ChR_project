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

Register positive events by the sensitive side of the detector

If 'followMinMaxValues' is defined in the 'DefsNConsts.hh' header,
the limits are tracked for further improving 'boostEfficiency' mode,
i.e., the limits for importance sampling used in the most basic form
in the StackingAction class
*/

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

class SteppingAction final : public G4UserSteppingAction
{
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
  static std::atomic<size_t> fNoOfDetections;
  SteppingAction_Messenger* fSteppingMessenger;
  G4RotationMatrix fRotToDetSystem; //passive rot
  G4ThreeVector fTrToDetSurfSystem;
  G4int fVerboseLevel;
};

//=======Set inlines=======
void SteppingAction::SetVerboseLevel(const G4int val)
{
  fVerboseLevel = val;
}

//=======Get inlines=======
G4int SteppingAction::GetVerboseLevel() const
{
  return fVerboseLevel;
}

size_t SteppingAction::GetNoOfDetections()
{
  return fNoOfDetections.load(std::memory_order_relaxed) - 1;
}

endChR

#endif // !SteppingAction_hh