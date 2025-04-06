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

Accessing physics data without copying them, but using references
*/

#pragma once
#ifndef G4AccessPhysicsVector_hh
#define G4AccessPhysicsVector_hh

//G4 headers
#include "globals.hh"
#include "G4PhysicsVector.hh"
//std:: headers
#include <algorithm>

/*
Unfortunately, G4PhysicsFreeVector doesn't have a virtual destructor
and thus I don't want to inherit it... though in this case it can be done.
Still, even inheriting G4PhysicsFreeVector would not solve the problem.
On the other hand, the problem here can be solved by casting (reinterpret) which will be done
*/
class G4AccessPhysicsVector : public G4PhysicsVector
{
public:
  G4AccessPhysicsVector() = default;
  virtual ~G4AccessPhysicsVector() override = default;
  //=======Get inlines=======
  [[nodiscard]] inline const std::vector<G4double>& GetBinVector() const; //energy vector
  [[nodiscard]] inline const std::vector<G4double>& GetDataVector() const; //crossection/energyloss
  [[nodiscard]] inline const std::vector<G4double>& GetSecDerivative() const; //second derivatives
  /*
  This class was created just to access vectors (for Cherenkov radiation) and write the following functions
  In the original class, instead of getting a real max/min, only the left/right-most value
  is returned. That means that if n(E) spectrum is a more complex function, one can't see it.
  In general, it would be better to call functions GetFrontValue() and GetBackValue(), not
  GetMinValue() and GetMaxValue(). That way there would be no confusion for sure.
  */
  [[nodiscard]] inline G4double GetRealDataVectorMax() const;
  [[nodiscard]] inline G4double GetRealDataVectorMin() const;
};

//=======Get inlines=======
const std::vector<G4double>& G4AccessPhysicsVector::GetBinVector() const
{
  return binVector;
}

const std::vector<G4double>& G4AccessPhysicsVector::GetDataVector() const
{
  return dataVector;
}

const std::vector<G4double>& G4AccessPhysicsVector::GetSecDerivative() const
{
  return secDerivative;
}

G4double G4AccessPhysicsVector::GetRealDataVectorMax() const
{
  return dataVector.at(static_cast<size_t>(std::max_element(dataVector.begin(), dataVector.end()) - dataVector.begin()));
}

G4double G4AccessPhysicsVector::GetRealDataVectorMin() const
{
  return dataVector.at(static_cast<size_t>(std::min_element(dataVector.begin(), dataVector.end()) - dataVector.begin()));
}

#endif // !G4AccessPhysicsVector_hh