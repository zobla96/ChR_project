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

Read the "DumpModelInfo" method
*/

#pragma once
#ifndef G4ThinTargetChR_Model_hh
#define G4ThinTargetChR_Model_hh

//G4 headers
#include "G4BaseChR_Model.hh"
//std:: headers
#include <unordered_map>

class G4VPhysicalVolume;
class G4AffineTransform;
class G4Box;

class G4ThinTargetChR_Model : public G4BaseChR_Model
{
public:
  G4ThinTargetChR_Model(const char* theName = "G4ThinTargetChR_Model");
  virtual ~G4ThinTargetChR_Model() override = default;

  G4ThinTargetChR_Model(const G4ThinTargetChR_Model&) = delete;
  G4ThinTargetChR_Model& operator=(const G4ThinTargetChR_Model&) = delete;
  G4ThinTargetChR_Model(G4ThinTargetChR_Model&&) /*noexcept*/ = delete;
  G4ThinTargetChR_Model& operator=(G4ThinTargetChR_Model&&) /*noexcept*/ = delete;

  [[nodiscard]] virtual G4VParticleChange* PostStepModelDoIt(const G4Track&, const G4Step&, const G4CherenkovMatData&) override;
  virtual void DumpModelInfo() const override;
  virtual void BuildModelPhysicsTable(const G4ParticleDefinition&) override;
private:
  // Helper methods to simplify read of the code
  G4bool FindParticleEntryAndExitPoints(
               G4ThreeVector& entryPoint,
               G4ThreeVector& exitPoint,
               const G4ThreeVector& localMiddlePoint,
               const G4ThreeVector& localDirection,
               const G4ThreeVector& localPrePoint,
               const G4CherenkovMatData& aChRMatData) const;
  G4double CalculateGaussSigmaDistance(
               const G4ThreeVector& entryPoint,
               const G4ThreeVector& exitPoint,
               const G4ThreeVector& photonDirection,
               const G4CherenkovMatData& matData) const;
  void SetBoxPhysicsTableParameters(G4CherenkovMatData&, const G4ThreeVector&) const;
};

#endif // !G4ThinTargetChR_Model_hh