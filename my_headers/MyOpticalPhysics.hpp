#pragma once
#ifndef MyOpticalPhysics_hpp
#define MyOpticalPhysics_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "CherenkovProcess.hpp"
//G4 headers
#include "G4VPhysicsConstructor.hh"
//...
#include "G4ProcessManager.hh"
#include "G4OpticalParameters.hh"
#include "G4OpticalPhoton.hh"
#include "G4LossTableManager.hh"
#include "G4BuilderType.hh"
//processes
#include "G4OpAbsorption.hh"
#include "G4OpRayleigh.hh"
#include "G4OpMieHG.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4OpWLS.hh"
#include "G4OpWLS2.hh"
#include "G4Scintillation.hh"

beginChR

//Class G4OpticalPhysics is not inheritence-safe so creating new OpticalPhysics
//All processes other than Cherenkov radiation will remain identical (with some initialziation changes)
class MyOpticalPhysics : public G4VPhysicsConstructor {
public:
	MyOpticalPhysics(int verbose);
	virtual ~MyOpticalPhysics() override;
	virtual void ConstructParticle() override; //MAKE SURE TO RUN THIS ONE AND INSTANTIATE OPTICALPARAMETERS!
	virtual void ConstructProcess() override;
protected:
	//if one wants to change any of the processes, just inherit the class and override some of the methods
	virtual void InstantiateOpticalParameters(); //must be kept outside the constructor so one could override
	virtual void LoadOpAbsorption();
	virtual void LoadOpRayleigh();
	virtual void LoadOpMieHG();
	virtual void LoadOpBoundaryProcess();
	virtual void LoadOpWLS();
	virtual void LoadOpWLS2();
	virtual void LoadCherenkov();
	virtual void LoadOpticalTransitionRad(); //for now does nothing
	virtual void LoadScintilation();
private:

};

endChR

#endif // !MyOpticalPhysics_hpp