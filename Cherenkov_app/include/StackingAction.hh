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

Determine what happens with various particles.
If 'boostEfficiency' is defined in the 'DefsNConsts.hh' header,
importance sampling is conducted. Without importance sampling, most
of Cherenkov photons just fly around and take processor cycles while
not being detected - a detector captures a very small solid angle to
observe quasi-monochromatic Cherenkov spectral lines instead of a
wide spectrum. Without 'boostEfficiency', one will need run that last
for hours/days in order to observe high-intensity spectral lines.
*/

#pragma once
#ifndef StackingAction_hh
#define StackingAction_hh

// user headers
#include "DefsNConsts.hh"
// G4 headers
#include "G4UserStackingAction.hh"
#include "G4AffineTransform.hh"

class G4Track;
class G4PhysicsFreeVector;

beginChR

class StackingAction final : public G4UserStackingAction
{
public:
  StackingAction();
  ~StackingAction() override = default;
  G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack) override;
#ifdef boostEfficiency
  // Set inlines
  inline void SetDeltaPhi(const G4double);
  inline void SetThetaMin(const G4double);
  inline void SetThetaMax(const G4double);
  // Get inlines
  [[nodiscard]] inline G4double GetDeltaPhi() const;
  [[nodiscard]] inline G4double GetThetaMin() const;
  [[nodiscard]] inline G4double GetThetaMax() const;
private:
  const G4RotationMatrix fRotationMatrix; // local to global
  const G4PhysicsFreeVector* fRindexVector = nullptr;
  G4double fDeltaPhi;
  G4double fThetaMin;
  G4double fThetaMax;
  G4bool fWithGaussSigma;
#endif // boostEfficiency
};

#ifdef boostEfficiency
// Set inlines
void StackingAction::SetDeltaPhi(const G4double value)
{
  fDeltaPhi = value;
}

void StackingAction::SetThetaMin(const G4double value)
{
  fThetaMin = value;
}

void StackingAction::SetThetaMax(const G4double value)
{
  fThetaMax = value;
}

// Get inlines
G4double StackingAction::GetDeltaPhi() const
{
  return fDeltaPhi;
}

G4double StackingAction::GetThetaMin() const
{
  return fThetaMin;
}

G4double StackingAction::GetThetaMax() const
{
  return fThetaMax;
}

#endif // boostEfficiency

endChR

#endif // !StackingAction_hh