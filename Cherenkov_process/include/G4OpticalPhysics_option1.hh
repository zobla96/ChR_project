//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

/*
ABOUT THE HEADER
----------------
A class used to load all optical processes. The class is similar to the
G4OpticalPhysics class, with the difference in virtual methods, including
the destructor (it's inheritance safe). Also, the class loads all
optical processes separately, so one could inherit it, and just override
a specific method to load possible user optical processes without
rewriting the entire optical physics.

The G4OpticalPhysics_option1 class loads only a standard Cherenkov radiation
process (G4StandardCherenkovProcess). That means the process is based on the
G4StandardChR_Model class, i.e., it can be adequately used only for "ideal
radiators". This was done for performance reasons, as most Geant4 applications
need to use only the stand Cherenkov (Frank-Tamm) theory. However, if one
needs to use other Cherenkov (user developed or thin-target) models, the
G4OpticalPhysics_option2 should be used.

NOTE1: The G4StandardCherenkovProcess class is very similar to the G4Cerenkov
class, but with somewhat improved physics tables.

NOTE2: all processes but for Cherenkov radiation are identical to the
G4OpticalPhysics processes - they are just loaded differently.
*/

#pragma once
#ifndef G4OpticalPhysics_option1_hh
#define G4OpticalPhysics_option1_hh

//G4 headers
#include "G4VPhysicsConstructor.hh"
//...
#include "globals.hh"


class G4OpticalPhysics_option1 : public G4VPhysicsConstructor {
public:
	G4OpticalPhysics_option1(G4int verbose = 0, const G4String& physicsName = "OpticalPhysics_op1");
	virtual ~G4OpticalPhysics_option1() override = default;
	virtual void ConstructParticle() override;
	virtual void ConstructProcess() override;
protected:
	//if one wants to change any of the processes, just inherit the class and override specific methods
	virtual void LoadOpAbsorption();
	virtual void LoadOpRayleigh();
	virtual void LoadOpMieHG();
	virtual void LoadOpBoundaryProcess();
	virtual void LoadOpWLS();
	virtual void LoadOpWLS2();
	virtual void LoadCherenkov();
	virtual void LoadOpticalTransitionRad(); //for now loads nothing - there's no such a process in Geant4
	virtual void LoadScintillation();
};

#endif // !G4OpticalPhysics_option1_hh