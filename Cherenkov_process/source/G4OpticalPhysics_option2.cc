//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#include "G4OpticalPhysics_option2.hh"
#include "G4CherenkovProcess.hh"
#include "G4OpticalParameters.hh"
#include "G4ProcessManager.hh"

//=========public G4OpticalPhysics_option2:: methods=========

G4OpticalPhysics_option2::G4OpticalPhysics_option2(G4int verbose, const G4String& physicsName)
:G4OpticalPhysics_option1(verbose, physicsName) {
	//no need to load G4ExtraOpticalParameters here - it's a singleton anyway
	
	//the following is just to help one search through the G4 source code
	//SetPhysicsType(bUnknown)
}

//=========protected G4OpticalPhysics_option2:: methods=========

void G4OpticalPhysics_option2::LoadCherenkov() {
	G4CherenkovProcess* cherenkov = new G4CherenkovProcess{};
	G4OpticalParameters* params = G4OpticalParameters::Instance();
	auto aParticleIterator = GetParticleIterator();
	aParticleIterator->reset();
	while ((*aParticleIterator)()) {
		G4ParticleDefinition* particle = aParticleIterator->value();
		G4ProcessManager* pManager = particle->GetProcessManager();
		if (!pManager) {
			G4ExceptionDescription ed;
			ed << "Particle " << particle->GetParticleName() << "without a Process Manager";
			G4Exception("G4OpticalPhysics_option2::LoadCherenkov", "FE_G4OptPhys_o2-01", FatalException, ed);
			return;  //just to make sure no compiler would complain
		}
		if (cherenkov->IsApplicable(*particle) && params->GetProcessActivation("Cerenkov")) {
			pManager->AddProcess(cherenkov);
			pManager->SetProcessOrdering(cherenkov, idxPostStep);
		}
	}
}