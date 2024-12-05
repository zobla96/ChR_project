//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#include "PhysicsList.hpp"

beginChR

//=========public ChR::PhysicsList:: methods=========

PhysicsList::PhysicsList(double gamma, double electron, double positron, double proton, int verbose)
:m_EMPhysics(UseElectromagnetic::G4EmStandardPhysics) {
	p_phListMessenger = new PhysicsList_Messenger{ this };
	verboseLevel = verbose;
	if (gamma <= 0.) m_radiatorRangeCuts_gamma = defaultCutValue;
	else m_radiatorRangeCuts_gamma = gamma;
	if (electron <= 0.) m_radiatorRangeCuts_electron = defaultCutValue;
	else m_radiatorRangeCuts_electron = electron;
	if (positron <= 0.) m_radiatorRangeCuts_positron = defaultCutValue;
	else m_radiatorRangeCuts_positron = positron;
	if (proton <= 0.) m_radiatorRangeCuts_proton = defaultCutValue;
	else m_radiatorRangeCuts_proton = proton;
	////Now registering physics
	p_theEMPhysics = new G4EmStandardPhysics{ verbose };
	RegisterPhysics(p_theEMPhysics);
	RegisterPhysics(new G4EmExtraPhysics{ verbose });
	RegisterPhysics(new G4DecayPhysics{ verbose });
	RegisterPhysics(new G4HadronElasticPhysics{ verbose });
	RegisterPhysics(new G4HadronInelasticQBBC{ verbose });
	RegisterPhysics(new G4StoppingPhysics{ verbose });
	RegisterPhysics(new G4IonPhysics{ verbose });
	//RegisterPhysics(new G4OpticalPhysics{ verbose });
	//RegisterPhysics(new G4OpticalPhysics_option1{ verbose });
	RegisterPhysics(new G4OpticalPhysics_option2{ verbose });
}

PhysicsList::~PhysicsList() {
	delete p_phListMessenger;
	//delete p_theEMPhysics; it's managed by G4VModularPhysicsList and it can't be changed in an illegal state
}

//void PhysicsList::ConstructProcess() {
//	
//}

//void PhysicsList::ConstructParticle() {
//
//}

void PhysicsList::SetCuts() {
	SetCutsWithDefault();
	//G4Region* worldRegion = G4RegionStore::GetInstance()->GetRegion("DefaultRegionForTheWorld");
	G4ProductionCuts* theCuts = new G4ProductionCuts{};
	theCuts->SetProductionCut(m_radiatorRangeCuts_gamma, 0); /*gamma*/
	theCuts->SetProductionCut(m_radiatorRangeCuts_electron, 1); /*e-*/
	theCuts->SetProductionCut(m_radiatorRangeCuts_positron, 2); /*e+*/
	theCuts->SetProductionCut(m_radiatorRangeCuts_proton, 3); /*proton*/
	// in this phase, there's still no instance of production cuts
	G4RegionStore::GetInstance()->GetRegion("radiatorRegion")->SetProductionCuts(theCuts);

	if (verboseLevel > 0)
		DumpCutValuesTable();
}

void PhysicsList::ChangeEMPhysics(UseElectromagnetic newEMPhysics) {
	if (G4StateManager::GetStateManager()->GetCurrentState() != G4State_PreInit) {
		const char* err = "You can change registered physics only during the PreInit phase - before running G4RunManager::Initialize()\n";
		G4Exception("PhysicsList::ChangeEMPhysics", "FE_PhysList01", FatalException, err);
		return;
	}
	if (m_EMPhysics == newEMPhysics) {
		std::ostringstream err;
		err << "Electromagnetic physics " << m_EMPhysics << " was already loaded!\n"
			<< "EM physics has not changed!\n";
		G4Exception("PhysicsList::ChangeEMPhysics", "WE_PhysList01", JustWarning, err);
		return;
	}
	if(verboseLevel > 0)
		std::cout << "Changing electromagnetic physics from " << m_EMPhysics << " to " << newEMPhysics << "!\n";

	RemovePhysics(p_theEMPhysics);
	delete p_theEMPhysics; // seems RemovePhysics doesn't delete the physics

	switch (newEMPhysics) {
	case ChR::UseElectromagnetic::G4EmStandardPhysics:
		p_theEMPhysics = new G4EmStandardPhysics{ verboseLevel };
		RegisterPhysics(p_theEMPhysics);
		m_EMPhysics = UseElectromagnetic::G4EmStandardPhysics;
		break;
	case ChR::UseElectromagnetic::G4EmStandardPhysics_option1:
		p_theEMPhysics = new G4EmStandardPhysics_option1{ verboseLevel };
		RegisterPhysics(p_theEMPhysics);
		m_EMPhysics = UseElectromagnetic::G4EmStandardPhysics_option1;
		break;
	case ChR::UseElectromagnetic::G4EmStandardPhysics_option2:
		p_theEMPhysics = new G4EmStandardPhysics_option2{ verboseLevel };
		RegisterPhysics(p_theEMPhysics);
		m_EMPhysics = UseElectromagnetic::G4EmStandardPhysics_option2;
		break;
	case ChR::UseElectromagnetic::G4EmStandardPhysics_option3:
		p_theEMPhysics = new G4EmStandardPhysics_option3{ verboseLevel };
		RegisterPhysics(p_theEMPhysics);
		m_EMPhysics = UseElectromagnetic::G4EmStandardPhysics_option3;
		break;
	case ChR::UseElectromagnetic::G4EmStandardPhysics_option4:
		p_theEMPhysics = new G4EmStandardPhysics_option4{ verboseLevel };
		RegisterPhysics(p_theEMPhysics);
		m_EMPhysics = UseElectromagnetic::G4EmStandardPhysics_option4;
		break;
	case ChR::UseElectromagnetic::G4EmLivermorePhysics:
		p_theEMPhysics = new G4EmLivermorePhysics{ verboseLevel };
		RegisterPhysics(p_theEMPhysics);
		m_EMPhysics = UseElectromagnetic::G4EmLivermorePhysics;
		break;
	case ChR::UseElectromagnetic::G4EmLowEPPhysics:
		p_theEMPhysics = new G4EmLowEPPhysics{ verboseLevel };
		RegisterPhysics(p_theEMPhysics);
		m_EMPhysics = UseElectromagnetic::G4EmLowEPPhysics;
		break;
	case ChR::UseElectromagnetic::G4EmPenelopePhysics:
		p_theEMPhysics = new G4EmPenelopePhysics{ verboseLevel };
		RegisterPhysics(p_theEMPhysics);
		m_EMPhysics = UseElectromagnetic::G4EmPenelopePhysics;
		break;
	case ChR::UseElectromagnetic::G4EmStandardPhysicsGS:
		p_theEMPhysics = new G4EmStandardPhysicsGS{ verboseLevel };
		RegisterPhysics(p_theEMPhysics);
		m_EMPhysics = UseElectromagnetic::G4EmStandardPhysicsGS;
		break;
	case ChR::UseElectromagnetic::G4EmStandardPhysicsSS:
		p_theEMPhysics = new G4EmStandardPhysicsSS{ verboseLevel };
		RegisterPhysics(p_theEMPhysics);
		m_EMPhysics = UseElectromagnetic::G4EmStandardPhysicsSS;
		break;
	case ChR::UseElectromagnetic::G4EmStandardPhysicsWVI:
		p_theEMPhysics = new G4EmStandardPhysicsWVI{ verboseLevel };
		RegisterPhysics(p_theEMPhysics);
		m_EMPhysics = UseElectromagnetic::G4EmStandardPhysicsWVI;
		break;
	default:
		G4Exception("PhysicsList::ChangeEMPhysics", "FE_PhysList02", FatalException, "This one should not have happened\n");
		break;
	}

	if(verboseLevel > 0)
		std::cout << "New electromagnetic physics " << m_EMPhysics << " has been registered!\n";
}

endChR