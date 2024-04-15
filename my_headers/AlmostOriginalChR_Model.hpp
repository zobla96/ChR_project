#pragma once
#ifndef AlmostOriginalChR_Model_hpp
#define AlmostOriginalChR_Model_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "BaseChR_Model.hpp"
//G4 headers
#include "G4Poisson.hh"
#include "G4OpticalPhoton.hh"

beginChR

class AlmostOriginalChR_Model : public BaseChR_Model {
public:
	AlmostOriginalChR_Model(const char* name = "AlmostOriginalChR_Model");
	virtual ~AlmostOriginalChR_Model() override;
	_NODISCARD virtual G4VParticleChange* PostStepModelDoIt(const G4Track&, const G4Step&) override;
	virtual void DumpModelInfo() const override;
private:
	
};

endChR

#endif // !AlmostOriginalChR_Model_hpp