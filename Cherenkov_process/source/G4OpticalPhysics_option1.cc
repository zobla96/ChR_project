//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//G4 headers
#include "G4OpticalPhysics_option1.hh"
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
#include "G4StandardCherenkovProcess.hh"
//end of processes

//=========public G4OpticalPhysics_option1:: methods=========

G4OpticalPhysics_option1::G4OpticalPhysics_option1(G4int verbose, const G4String& physicsName)
: G4VPhysicsConstructor(physicsName) {
	verboseLevel = verbose;
	SetPhysicsType(bUnknown); //a default one, but writing it helps in searching
}

void G4OpticalPhysics_option1::ConstructParticle() {
	G4OpticalPhoton::Definition();
}

void G4OpticalPhysics_option1::ConstructProcess() {
	if (G4OpticalPhoton::OpticalPhoton()->GetProcessManager() == nullptr) {
		std::string errorMsg = "G4OpticalPhoton still has a nullptr G4ProcessManager\n";
		G4Exception("G4OpticalPhysics_option1::ConstructProcess", "FE_G4OptPhys_o1-01", FatalException, errorMsg.c_str());
	}
	LoadOpAbsorption();
	LoadOpRayleigh();
	LoadOpMieHG();
	LoadOpBoundaryProcess();
	LoadOpWLS();
	LoadOpWLS2();
	LoadCherenkov();
	LoadOpticalTransitionRad();
	LoadScintillation();
}

//=========protected G4OpticalPhysics_option1:: methods=========

void G4OpticalPhysics_option1::LoadOpAbsorption() {
	if (G4OpticalParameters::Instance()->GetProcessActivation("OpAbsorption"))
		G4OpticalPhoton::OpticalPhoton()->GetProcessManager()->AddDiscreteProcess(new G4OpAbsorption{});
}

void G4OpticalPhysics_option1::LoadOpRayleigh() {
	if (G4OpticalParameters::Instance()->GetProcessActivation("OpRayleigh"))
		G4OpticalPhoton::OpticalPhoton()->GetProcessManager()->AddDiscreteProcess(new G4OpRayleigh{});
}

void G4OpticalPhysics_option1::LoadOpMieHG() {
	if (G4OpticalParameters::Instance()->GetProcessActivation("OpMieHG"))
		G4OpticalPhoton::OpticalPhoton()->GetProcessManager()->AddDiscreteProcess(new G4OpMieHG{});
}

void G4OpticalPhysics_option1::LoadOpBoundaryProcess() {
	if (G4OpticalParameters::Instance()->GetProcessActivation("OpBoundary"))
		G4OpticalPhoton::OpticalPhoton()->GetProcessManager()->AddDiscreteProcess(new G4OpBoundaryProcess{});
}

void G4OpticalPhysics_option1::LoadOpWLS() {
	if (G4OpticalParameters::Instance()->GetProcessActivation("OpWLS"))
		G4OpticalPhoton::OpticalPhoton()->GetProcessManager()->AddDiscreteProcess(new G4OpWLS{});
}

void G4OpticalPhysics_option1::LoadOpWLS2() {
	if (G4OpticalParameters::Instance()->GetProcessActivation("OpWLS2"))
		G4OpticalPhoton::OpticalPhoton()->GetProcessManager()->AddDiscreteProcess(new G4OpWLS2{});
}

void G4OpticalPhysics_option1::LoadCherenkov() {
	G4StandardCherenkovProcess* cherenkov = new G4StandardCherenkovProcess{};
	G4OpticalParameters* params = G4OpticalParameters::Instance();
	auto aParticleIterator = GetParticleIterator();
	aParticleIterator->reset();
	while ((*aParticleIterator)()) {
		G4ParticleDefinition* particle = aParticleIterator->value();
		G4ProcessManager* pManager = particle->GetProcessManager();
		if (!pManager) {
			G4ExceptionDescription ed;
			ed << "Particle " << particle->GetParticleName() << "without a Process Manager";
			G4Exception("G4OpticalPhysics_option1::LoadCherenkov", "FE_G4OptPhys_o1-02", FatalException, ed);
			return;  //just to make sure no compiler would complain
		}
		if (cherenkov->IsApplicable(*particle) && params->GetProcessActivation("Cerenkov")) {
			pManager->AddProcess(cherenkov);
			pManager->SetProcessOrdering(cherenkov, idxPostStep);
		}
	}
}

void G4OpticalPhysics_option1::LoadOpticalTransitionRad() {
	//for now there's no such a process
}

void G4OpticalPhysics_option1::LoadScintillation() {
	G4Scintillation* scint = new G4Scintillation{};
	G4EmSaturation* emSaturation = G4LossTableManager::Instance()->EmSaturation();
	G4OpticalParameters* params = G4OpticalParameters::Instance();
	scint->AddSaturation(emSaturation);
	auto aParticleIterator = GetParticleIterator();
	aParticleIterator->reset();
	while ((*aParticleIterator)()) {
		G4ParticleDefinition* particle = aParticleIterator->value();
		G4ProcessManager* pManager = particle->GetProcessManager();
		if (!pManager) {
			G4ExceptionDescription ed;
			ed << "Particle " << particle->GetParticleName() << "without a Process Manager";
			G4Exception("G4OpticalPhysics_option1::LoadScintillation()", "FE_G4OptPhys_o1-03", FatalException, ed);
			return;  //just to make sure no compiler would complain
		}
		if (scint->IsApplicable(*particle) &&
			params->GetProcessActivation("Scintillation")) {
			pManager->AddProcess(scint);
			pManager->SetProcessOrderingToLast(scint, idxAtRest);
			pManager->SetProcessOrderingToLast(scint, idxPostStep);
		}
	}
}