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

Generate the primaries with a necessary distribution
*/

#pragma once
#ifndef PrimaryGeneratorAction_hh
#define PrimaryGeneratorAction_hh

// user headers
#include "DefsNConsts.hh"
#include "PrimaryGeneratorAction_Messenger.hh"
// G4 headers
#include "G4VUserPrimaryGeneratorAction.hh"

class G4ParticleGun;

beginChR

class PrimaryGeneratorAction_Messenger;

class PrimaryGeneratorAction final : public G4VUserPrimaryGeneratorAction
{
public:
  PrimaryGeneratorAction();
  ~PrimaryGeneratorAction() override;
  void GeneratePrimaries(G4Event*) override;
  //=======Set inlines=======
  inline void SetBeamSigma(const G4double);
  inline void SetDistanceZ(const G4double);
  inline void SetDivergenceSigma(const G4double);
  //=======Get inlines=======
  [[nodiscard]] inline G4double GetBeamSigma() const;
  [[nodiscard]] inline G4double GetDivSigma() const;
  [[nodiscard]] inline G4double GetDistanceZ() const; // it's an absolute value while it should be negative
  [[nodiscard]] inline G4ParticleGun* GetParticleGun() const; // non-const gun for simplicity
private:
  G4ParticleGun* fTheGun;
  PrimaryGeneratorAction_Messenger* fPGeneratorMessenger;
  G4double fBeamSigma;
  G4double fSinBeamDivergenceTheta;
  G4double fZDistance;
};

//=======Set inlines=======
void PrimaryGeneratorAction::SetBeamSigma(const G4double value)
{
  fBeamSigma = value;
}

void PrimaryGeneratorAction::SetDistanceZ(const G4double value)
{
  fZDistance = value;
}

void PrimaryGeneratorAction::SetDivergenceSigma(const G4double val)
{
  fSinBeamDivergenceTheta = std::sin(val);
}

//=======Get inlines=======
G4double PrimaryGeneratorAction::GetBeamSigma() const
{
  return fBeamSigma;
}

G4double PrimaryGeneratorAction::GetDivSigma() const
{
  return std::asin(fSinBeamDivergenceTheta);
}

G4double PrimaryGeneratorAction::GetDistanceZ() const
{
  return fZDistance;
}

G4ParticleGun* PrimaryGeneratorAction::GetParticleGun() const
{
  return fTheGun;
}

endChR

#endif // !PrimaryGeneratorAction_hh