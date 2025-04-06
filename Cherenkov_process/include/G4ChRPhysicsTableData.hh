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

The data needed to process any kind of refractive-index dependency according
to the Frank-Tamm theory.

See the description in the 'G4BaseChR_Model.hh' header for more information
*/

#pragma once
#ifndef G4ChRPhysicsTableData_hh
#define G4ChRPhysicsTableData_hh

//G4 headers
#include "globals.hh"
#include "G4ThreeVector.hh"
//std:: headers
#include <vector>
#include <utility>

struct G4ChRPhysTableData
{
  struct G4AroundBetaBasedValues;
  // going twice with std::vector<...>* as it should take less memory in most of applications, i.e.,
  // mostly CDF values are not needed! A vector itself is 24 bytes.
  std::vector<G4AroundBetaBasedValues> fAroundBetaValues;
  std::vector<G4ThreeVector>* fBigBetaCDFVector = nullptr; // leftInt and rightInt values for CDF, but beta is not included yet
  //used only for maxBeta

  struct G4AroundBetaBasedValues
  {
    G4AroundBetaBasedValues(G4double beta, G4double leftInt, G4double rightInt)
    : fBetaValue(beta),
      fLeftIntegralValue(leftInt),
      fRightIntegralValue(rightInt)
    {}

    ~G4AroundBetaBasedValues() { delete fValuesCDF; }

    G4double fBetaValue;
    G4double fLeftIntegralValue;
    G4double fRightIntegralValue;
    std::vector<std::pair<G4double, G4double>>* fValuesCDF = nullptr; // real cdf values in range [0, 1]
  };

  G4ChRPhysTableData() = default;
  ~G4ChRPhysTableData() { delete fBigBetaCDFVector; }

  G4ChRPhysTableData& operator=(const G4ChRPhysTableData& other) = delete;
  G4ChRPhysTableData(const G4ChRPhysTableData& other) = delete;

  G4ChRPhysTableData& operator=(G4ChRPhysTableData&& other) noexcept
  {
    if (this == &other)
      return *this;
    fAroundBetaValues = std::exchange(other.fAroundBetaValues, std::vector<G4AroundBetaBasedValues>{});
    delete fBigBetaCDFVector;
    fBigBetaCDFVector = std::exchange(other.fBigBetaCDFVector, nullptr);
    return *this;
  }

  G4ChRPhysTableData(G4ChRPhysTableData&& other) noexcept
  : fAroundBetaValues(std::exchange(other.fAroundBetaValues, std::vector<G4AroundBetaBasedValues>{})),
    fBigBetaCDFVector(std::exchange(other.fBigBetaCDFVector, nullptr)) {}

};

#endif // !G4ChRPhysicsTableData_hh