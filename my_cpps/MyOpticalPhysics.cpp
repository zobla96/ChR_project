#include "MyOpticalPhysics.hpp"

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

beginChR

//=========public ChR::MyOpticalPhysics:: methods=========
MyOpticalPhysics::MyOpticalPhysics(int verbose)
: G4VPhysicsConstructor("MyOpticalPhysics") {
	verboseLevel = verbose;
	SetPhysicsType(bUnknown); //a default one, but writing it helps in searching
}

MyOpticalPhysics::~MyOpticalPhysics() {

}

void MyOpticalPhysics::ConstructParticle() {
	InstantiateOpticalParameters();
	//If you are adding some addition Cherenkov processes, pay attention to this method!! It doesn't load
	//any particles but allows the user to register MyOpticalParameters with his/her own <T> template parameter
	G4OpticalPhoton::Definition();
}

void MyOpticalPhysics::ConstructProcess() {
	if (G4OpticalPhoton::OpticalPhoton()->GetProcessManager() == nullptr) {
		std::string errorMsg = "G4OpticalPhoton still has a nullptr G4ProcessManager\n";
		G4Exception("ChR::MyOpticalPhysics::ConstructProcess()", "FE1013", FatalException, errorMsg.c_str());
	}
	LoadOpAbsorption();
	LoadOpRayleigh();
	LoadOpMieHG();
	LoadOpBoundaryProcess();
	LoadOpWLS();
	LoadOpWLS2();
	LoadCherenkov();
	LoadOpticalTransitionRad();
	LoadScintilation();
}

//=========protected ChR::MyOpticalPhysics:: methods=========
void MyOpticalPhysics::InstantiateOpticalParameters() {
	MyOpticalParameters<ChRModelIndex>::GetInstance();
}

void MyOpticalPhysics::LoadOpAbsorption() {
	if (G4OpticalParameters::Instance()->GetProcessActivation("OpAbsorption"))
		G4OpticalPhoton::OpticalPhoton()->GetProcessManager()->AddDiscreteProcess(new G4OpAbsorption{});
}

void MyOpticalPhysics::LoadOpRayleigh() {
	if (G4OpticalParameters::Instance()->GetProcessActivation("OpRayleigh"))
		G4OpticalPhoton::OpticalPhoton()->GetProcessManager()->AddDiscreteProcess(new G4OpRayleigh{});
}

void MyOpticalPhysics::LoadOpMieHG() {
	if (G4OpticalParameters::Instance()->GetProcessActivation("OpMieHG"))
		G4OpticalPhoton::OpticalPhoton()->GetProcessManager()->AddDiscreteProcess(new G4OpMieHG{});
}

void MyOpticalPhysics::LoadOpBoundaryProcess() {
	if (G4OpticalParameters::Instance()->GetProcessActivation("OpBoundary"))
		G4OpticalPhoton::OpticalPhoton()->GetProcessManager()->AddDiscreteProcess(new G4OpBoundaryProcess{});
}

void MyOpticalPhysics::LoadOpWLS() {
	if (G4OpticalParameters::Instance()->GetProcessActivation("OpWLS"))
		G4OpticalPhoton::OpticalPhoton()->GetProcessManager()->AddDiscreteProcess(new G4OpWLS{});
}

void MyOpticalPhysics::LoadOpWLS2() {
	if (G4OpticalParameters::Instance()->GetProcessActivation("OpWLS2"))
		G4OpticalPhoton::OpticalPhoton()->GetProcessManager()->AddDiscreteProcess(new G4OpWLS2{});
}

void MyOpticalPhysics::LoadScintilation() {
	G4Scintillation* scint = new G4Scintillation{};
	G4EmSaturation* emSaturation = G4LossTableManager::Instance()->EmSaturation();
	G4OpticalParameters* params = G4OpticalParameters::Instance();
	scint->AddSaturation(emSaturation);
	auto myParticleIterator = GetParticleIterator();
	myParticleIterator->reset();

	while ((*myParticleIterator)()) {
		G4ParticleDefinition* particle = myParticleIterator->value();
		G4String particleName = particle->GetParticleName();
		G4ProcessManager* pManager = particle->GetProcessManager();
		if (!pManager) {
			G4ExceptionDescription ed;
			ed << "Particle " << particleName << "without a Process Manager";
			G4Exception("ChR::G4OpticalPhysics::ConstructProcess()", "FE1014", FatalException, ed);
			return;  // else coverity complains for pManager use below
		}
		if (scint->IsApplicable(*particle) &&
			params->GetProcessActivation("Scintillation")) {
			pManager->AddProcess(scint);
			pManager->SetProcessOrderingToLast(scint, idxAtRest);
			pManager->SetProcessOrderingToLast(scint, idxPostStep);
		}
	}
}

void MyOpticalPhysics::LoadCherenkov() { //the same as G4OpticalPhysics with other process loaded
	CherenkovProcess<ChRModelIndex>* cherenkov = new CherenkovProcess<ChRModelIndex>{};
	G4OpticalParameters* params = G4OpticalParameters::Instance();
	auto myParticleIterator = GetParticleIterator();
	myParticleIterator->reset();
	while ((*myParticleIterator)()) {
		G4ParticleDefinition* particle = myParticleIterator->value();
		G4String particleName = particle->GetParticleName();

		G4ProcessManager* pManager = particle->GetProcessManager();
		if (!pManager) {
			G4ExceptionDescription ed;
			ed << "Particle " << particleName << "without a Process Manager";
			G4Exception("ChR::G4OpticalPhysics::ConstructProcess()", "FE1015", FatalException, ed);
			return;  // else coverity complains for pManager use below
		}

		if (cherenkov->IsApplicable(*particle) && params->GetProcessActivation("Cerenkov")) {
			pManager->AddProcess(cherenkov);
			pManager->SetProcessOrdering(cherenkov, idxPostStep);
		}
	}
}

void MyOpticalPhysics::LoadOpticalTransitionRad() {
	//for now there's no such a process
	//might add it in the future through the PCM or some other model
}

endChR