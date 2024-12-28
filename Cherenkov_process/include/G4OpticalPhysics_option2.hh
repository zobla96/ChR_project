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

The G4OpticalPhysics_option2 class inherits from G4OpticalPhysics_option1
and loads another Cherenkov radiation process - G4CherenkovProcess. This
process allows one to include any number of Cherenkov radiation models
that can be applied for different study cases.
*/

#ifndef G4OpticalPhysics_option2_hh
#define G4OpticalPhysics_option2_hh

#include "G4OpticalPhysics_option1.hh"

class G4OpticalPhysics_option2 : public G4OpticalPhysics_option1 {
public:
	G4OpticalPhysics_option2(G4int verbose = 0, const G4String& physicsName = "OpticalPhysics_op2");
	virtual ~G4OpticalPhysics_option2() override = default;
protected:
	virtual void LoadCherenkov() override;
};

#endif // !G4OpticalPhysics_option2_hh