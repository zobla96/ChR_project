//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

// user headers
#include "PhysicsList.hh"
// G4 headers
#include "G4RegionStore.hh"
#include "G4StateManager.hh"
// G4BuilderType::bUnknown
// optical physics ->
#include "G4OpticalPhysics.hh"
#include "G4OpticalPhysics_option1.hh"
#include "G4OpticalPhysics_option2.hh"
// G4BuilderType::bElectromagnetic
//#include "G4EmDNAPhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4EmStandardPhysics_option1.hh"
#include "G4EmStandardPhysics_option2.hh"
#include "G4EmStandardPhysics_option3.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4EmLivermorePhysics.hh"
#include "G4EmLowEPPhysics.hh"
#include "G4EmPenelopePhysics.hh"
#include "G4EmStandardPhysicsGS.hh"
#include "G4EmStandardPhysicsSS.hh"
#include "G4EmStandardPhysicsWVI.hh"
// G4BuilderType::bEmExtra
#include "G4EmExtraPhysics.hh"
// G4BuilderType::bDecay
#include "G4DecayPhysics.hh"
// G4BuilderType::bHadronElastic
#include "G4HadronElasticPhysics.hh"
// G4BuilderType::bHadronInelastic
#include "G4HadronInelasticQBBC.hh"
// G4BuilderType::bStopping
#include "G4StoppingPhysics.hh"
// G4BuilderType::bIons
#include "G4IonPhysics.hh"

// std:: headers
#include "iomanip"

beginChR

//=========public ChR::PhysicsList:: methods=========

PhysicsList::PhysicsList(G4int verbose, G4double gamma, G4double electron, G4double positron, G4double proton)
:m_EMPhysics(UseElectromagnetic::G4EmStandardPhysics),
m_optical(UseOptical::G4OpticalPhysics_option2) {
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
	// registering physics
	p_theEMPhysics = new G4EmStandardPhysics{ verbose };
	RegisterPhysics(p_theEMPhysics);
	RegisterPhysics(new G4EmExtraPhysics{ verbose });
	RegisterPhysics(new G4DecayPhysics{ verbose });
	RegisterPhysics(new G4HadronElasticPhysics{ verbose });
	RegisterPhysics(new G4HadronInelasticQBBC{ verbose });
	RegisterPhysics(new G4StoppingPhysics{ verbose });
	RegisterPhysics(new G4IonPhysics{ verbose });

	// registering optical - G4OpticalPhysics_option2 as default
	p_opticalPhysics = new G4OpticalPhysics_option2{ verbose };
	RegisterPhysics(p_opticalPhysics);
}

PhysicsList::~PhysicsList() {
	delete p_phListMessenger;
	// delete p_theEMPhysics; it's managed by G4VModularPhysicsList and it can't be changed in an illegal state
	// delete p_opticalPhysics; it's managed by G4VModularPhysicsList and it can't be changed in an illegal state
}

//void PhysicsList::ConstructProcess() {
//	
//}

//void PhysicsList::ConstructParticle() {
//
//}

void PhysicsList::SetCuts() {
	SetCutsWithDefault();
	// G4Region* worldRegion = G4RegionStore::GetInstance()->GetRegion("DefaultRegionForTheWorld");
	G4Region* radiatorRegion = G4RegionStore::GetInstance()->GetRegion("radiatorRegion");
	if (auto* theCuts = radiatorRegion->GetProductionCuts(); theCuts) {
		theCuts->SetProductionCut(m_radiatorRangeCuts_gamma, 0); /*gamma*/
		theCuts->SetProductionCut(m_radiatorRangeCuts_electron, 1); /*e-*/
		theCuts->SetProductionCut(m_radiatorRangeCuts_positron, 2); /*e+*/
		theCuts->SetProductionCut(m_radiatorRangeCuts_proton, 3); /*proton*/
	}
	else { // else should always be executed, but just in case...
		theCuts = new G4ProductionCuts{};
		theCuts->SetProductionCut(m_radiatorRangeCuts_gamma, 0); /*gamma*/
		theCuts->SetProductionCut(m_radiatorRangeCuts_electron, 1); /*e-*/
		theCuts->SetProductionCut(m_radiatorRangeCuts_positron, 2); /*e+*/
		theCuts->SetProductionCut(m_radiatorRangeCuts_proton, 3); /*proton*/
		radiatorRegion->SetProductionCuts(theCuts);
	}

	if (verboseLevel > 0)
		DumpCutValuesTable();
}

void PhysicsList::ChangeEMPhysics(const UseElectromagnetic newEMPhysics) {
	if (G4StateManager::GetStateManager()->GetCurrentState() != G4State_PreInit) {
		const char* err = "You can change registered physics only during the PreInit phase - before running G4RunManager::Initialize()\n";
		G4Exception("PhysicsList::ChangeEMPhysics", "FE_PhysList01", FatalException, err);
		return;
	}
	if (m_EMPhysics == newEMPhysics) {
		std::ostringstream err;
		err << "Electromagnetic physics \"" << m_EMPhysics << "\" was already loaded!\n"
			<< "EM physics has not changed!\n";
		G4Exception("PhysicsList::ChangeEMPhysics", "WE_PhysList01", JustWarning, err);
		return;
	}
	if(verboseLevel > 0)
		G4cout << "Changing electromagnetic physics from " << m_EMPhysics << " to " << newEMPhysics << "!\n";

	RemovePhysics(p_theEMPhysics);
	delete p_theEMPhysics; // RemovePhysics doesn't delete the physics

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
		G4cout << "New electromagnetic physics " << m_EMPhysics << " has been registered!\n";
}

void PhysicsList::ChangeOpticalPhysics(const UseOptical newOpticalPhysics) {
	if (G4StateManager::GetStateManager()->GetCurrentState() != G4State_PreInit) {
		const char* err = "You can change registered physics only during the PreInit phase - before running G4RunManager::Initialize()\n";
		G4Exception("PhysicsList::ChangeOpticalPhysics", "FE_PhysList03", FatalException, err);
		return;
	}
	if (m_optical == newOpticalPhysics) {
		std::ostringstream err;
		err << "Optical physics \"" << m_optical << "\" was already loaded!\n"
			<< "Optical physics has not changed!\n";
		G4Exception("PhysicsList::ChangeOpticalPhysics", "WE_PhysList02", JustWarning, err);
		return;
	}

	if (verboseLevel > 0)
		G4cout << "Changing electromagnetic physics from " << m_optical << " to " << newOpticalPhysics << "!\n";

	RemovePhysics(p_opticalPhysics);
	delete p_opticalPhysics; // RemovePhysics doesn't delete the physics

	switch (newOpticalPhysics) {
	case ChR::UseOptical::G4OpticalPhysics:
		p_opticalPhysics = new G4OpticalPhysics{ verboseLevel };
		RegisterPhysics(p_opticalPhysics);
		m_optical = UseOptical::G4OpticalPhysics;
		break;
	case ChR::UseOptical::G4OpticalPhysics_option1:
		p_opticalPhysics = new G4OpticalPhysics_option1{ verboseLevel };
		RegisterPhysics(p_opticalPhysics);
		m_optical = UseOptical::G4OpticalPhysics_option1;
		break;
	case ChR::UseOptical::G4OpticalPhysics_option2:
		p_opticalPhysics = new G4OpticalPhysics_option2{ verboseLevel };
		RegisterPhysics(p_opticalPhysics);
		m_optical = UseOptical::G4OpticalPhysics_option2;
		break;
	default: // how??
		G4Exception("PhysicsList::ChangeOpticalPhysics", "FE_PhysList04", FatalException, "This one should not have happened\n");
		break;
	}

	if (verboseLevel > 0)
		G4cout << "New electromagnetic physics " << m_optical << " has been registered!\n";
}

endChR