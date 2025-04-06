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

Used to save primary-particle data for boostEfficiency runs - used in StackingAction
*/

#pragma once
#ifndef TrackingAction_hh
#define TrackingAction_hh

//User built headers
#include "PrimaryGeneratorAction.hh"
//G4 headers
#include "G4UserTrackingAction.hh"
#include "G4Track.hh"

beginChR

struct SpecificTrackData
{
  SpecificTrackData() = default;
  ~SpecificTrackData() = default;
  SpecificTrackData(const SpecificTrackData&) = default;
  SpecificTrackData& operator=(const SpecificTrackData&) = default;

  G4double fBeta = 0.;
  G4ThreeVector fGlobalDirection{};
#ifdef followMinMaxValues
  G4double fPhiValue = DBL_MAX;
  G4double fThetaValue = DBL_MAX;
#endif // followMinMaxValues

};

class TrackingAction final : public G4UserTrackingAction
{
public:
  TrackingAction() = default;
  ~TrackingAction() override = default;
  void PreUserTrackingAction(const G4Track*) override;
  //void PostUserTrackingAction(const G4Track*) override;
  //=======Set inlines=======
  inline void SetBeta(const G4Track*, const G4double);
  inline void SetBeta(const G4int trackID, const G4double);
  inline void SetGlobalDirection(const G4Track*, const G4ThreeVector&);
  inline void SetGlobalDirection(const G4int trackID, const G4ThreeVector&);
#ifdef followMinMaxValues
  inline void SetPhiValue(const G4Track*, const G4double);
  inline void SetPhiValue(const G4int trackID, const G4double);
  inline void SetThetaValue(const G4Track*, const G4double);
  inline void SetThetaValue(const G4int trackID, const G4double);
#endif // followMinMaxValues
  //=======Get inlines=======
  [[nodiscard]] inline G4double GetBeta(const G4Track*) const;
  [[nodiscard]] inline G4double GetBeta(const G4int trackID) const;
  [[nodiscard]] inline const G4ThreeVector& GetGlobalDirection(const G4Track*) const;
  [[nodiscard]] inline const G4ThreeVector& GetGlobalDirection(const G4int trackID) const;
#ifdef followMinMaxValues
  [[nodiscard]] inline G4double GetPhiValue(const G4Track*) const;
  [[nodiscard]] inline G4double GetPhiValue(const G4int trackID) const;
  [[nodiscard]] inline G4double GetThetaValue(const G4Track*) const;
  [[nodiscard]] inline G4double GetThetaValue(const G4int trackID) const;
#endif // followMinMaxValues
  [[nodiscard]] inline SpecificTrackData& GetOrCreateSpecificTrackData(const G4Track*);
  [[nodiscard]] inline SpecificTrackData& GetOrCreateSpecificTrackData(const G4int trackID);
  // nullptr if there's no such data
  [[nodiscard]] inline SpecificTrackData* GetSpecificTrackData(const G4Track*);
  [[nodiscard]] inline SpecificTrackData* GetSpecificTrackData(const G4int trackID);
private:
  std::map<G4int, SpecificTrackData> fSpecificTrackDataMap;
};

//=======Set inlines=======
void TrackingAction::SetBeta(const G4Track* aTrack, const G4double value)
{
  fSpecificTrackDataMap.at(aTrack->GetTrackID()).fBeta = value;
}

void TrackingAction::SetBeta(const G4int trackID, const G4double value)
{
  fSpecificTrackDataMap.at(trackID).fBeta = value;
}

void TrackingAction::SetGlobalDirection(const G4Track* aTrack, const G4ThreeVector& value)
{
  fSpecificTrackDataMap.at(aTrack->GetTrackID()).fGlobalDirection = value.unit();
}

void TrackingAction::SetGlobalDirection(const G4int trackID, const G4ThreeVector& value)
{
  fSpecificTrackDataMap.at(trackID).fGlobalDirection = value.unit();
}

#ifdef followMinMaxValues
void TrackingAction::SetPhiValue(const G4Track* aTrack, const G4double value)
{
  fSpecificTrackDataMap.at(aTrack->GetTrackID()).fPhiValue = value;
}

void TrackingAction::SetPhiValue(const G4int trackID, const G4double value)
{
  fSpecificTrackDataMap.at(trackID).fPhiValue = value;
}

void TrackingAction::SetThetaValue(const G4Track* aTrack, const G4double value)
{
  fSpecificTrackDataMap.at(aTrack->GetTrackID()).fThetaValue = value;
}

void TrackingAction::SetThetaValue(const G4int trackID, const G4double value)
{
  fSpecificTrackDataMap.at(trackID).fThetaValue = value;
}

#endif // followMinMaxValues

//=======Get inlines=======
G4double TrackingAction::GetBeta(const G4Track* aTrack) const
{
  return fSpecificTrackDataMap.at(aTrack->GetTrackID()).fBeta;
}

G4double TrackingAction::GetBeta(const G4int trackID) const
{
  return fSpecificTrackDataMap.at(trackID).fBeta;
}

const G4ThreeVector& TrackingAction::GetGlobalDirection(const G4Track* aTrack) const
{
  return fSpecificTrackDataMap.at(aTrack->GetTrackID()).fGlobalDirection;
}

const G4ThreeVector& TrackingAction::GetGlobalDirection(const G4int trackID) const
{
  return fSpecificTrackDataMap.at(trackID).fGlobalDirection;
}

#ifdef followMinMaxValues
G4double TrackingAction::GetPhiValue(const G4Track* aTrack) const
{
  return fSpecificTrackDataMap.at(aTrack->GetTrackID()).fPhiValue;
}

G4double TrackingAction::GetPhiValue(const G4int trackID) const
{
  return fSpecificTrackDataMap.at(trackID).fPhiValue;
}

G4double TrackingAction::GetThetaValue(const G4Track* aTrack) const
{
  return fSpecificTrackDataMap.at(aTrack->GetTrackID()).fThetaValue;
}

G4double TrackingAction::GetThetaValue(const G4int trackID) const
{
  return fSpecificTrackDataMap.at(trackID).fThetaValue;
}

#endif // followMinMaxValues

SpecificTrackData& TrackingAction::GetOrCreateSpecificTrackData(const G4Track* aTrack)
{
  return fSpecificTrackDataMap[aTrack->GetTrackID()];
}

SpecificTrackData& TrackingAction::GetOrCreateSpecificTrackData(const G4int trackID)
{
  return fSpecificTrackDataMap[trackID];
}

SpecificTrackData* TrackingAction::GetSpecificTrackData(const G4Track* aTrack)
{
  try {
    return &fSpecificTrackDataMap.at(aTrack->GetTrackID());
  }
  catch (std::out_of_range) {
    return nullptr;
  }
}

SpecificTrackData* TrackingAction::GetSpecificTrackData(const G4int trackID)
{
  try {
    return &fSpecificTrackDataMap.at(trackID);
  }
  catch (std::out_of_range) {
    return nullptr;
  }
}

endChR

#endif // !TrackingAction_hh