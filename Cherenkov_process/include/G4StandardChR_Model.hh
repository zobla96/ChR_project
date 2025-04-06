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
#ifndef G4StandardChR_Model_hh
#define G4StandardChR_Model_hh

//G4 headers
#include "G4BaseChR_Model.hh"

class G4StandardChR_Model : public G4BaseChR_Model
{
public:
  G4StandardChR_Model(const char* theName = "G4StandardChR_Model");
  virtual ~G4StandardChR_Model() override = default;

  G4StandardChR_Model(const G4StandardChR_Model&) = delete;
  G4StandardChR_Model& operator=(const G4StandardChR_Model&) = delete;
  G4StandardChR_Model(G4StandardChR_Model&&) /*noexcept*/ = delete;
  G4StandardChR_Model& operator=(G4StandardChR_Model&&) /*noexcept*/ = delete;

  [[nodiscard]] virtual G4VParticleChange* PostStepModelDoIt(const G4Track&, const G4Step&, const G4CherenkovMatData&) override;
  virtual void DumpModelInfo() const override;
};

#endif // !G4StandardChR_Model_hh