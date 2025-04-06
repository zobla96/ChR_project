//##########################################
//#######        VERSION 1.1.0       #######
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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//=========public ChR::PhysicsList:: methods=========
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PhysicsList::PhysicsList(G4int verbose, G4double gamma, G4double electron, G4double positron, G4double proton)
: fTheEMPhysics(new G4EmStandardPhysics{ verbose }),
  fOpticalPhysics(new G4OpticalPhysics_option2{ verbose }),
  fPhListMessenger(new PhysicsList_Messenger{ this }),
  fEMPhysics(UseElectromagnetic::G4EmStandardPhysics),
  fOptical(UseOptical::G4OpticalPhysics_option2)
{

  verboseLevel = verbose;
  if (gamma <= 0.) fRadiatorRangeCuts_gamma = defaultCutValue;
  else fRadiatorRangeCuts_gamma = gamma;
  if (electron <= 0.) fRadiatorRangeCuts_electron = defaultCutValue;
  else fRadiatorRangeCuts_electron = electron;
  if (positron <= 0.) fRadiatorRangeCuts_positron = defaultCutValue;
  else fRadiatorRangeCuts_positron = positron;
  if (proton <= 0.) fRadiatorRangeCuts_proton = defaultCutValue;
  else fRadiatorRangeCuts_proton = proton;
  // registering physics
  RegisterPhysics(fTheEMPhysics);
  RegisterPhysics(new G4EmExtraPhysics{ verbose });
  RegisterPhysics(new G4DecayPhysics{ verbose });
  RegisterPhysics(new G4HadronElasticPhysics{ verbose });
  RegisterPhysics(new G4HadronInelasticQBBC{ verbose });
  RegisterPhysics(new G4StoppingPhysics{ verbose });
  RegisterPhysics(new G4IonPhysics{ verbose });

  // registering optical - G4OpticalPhysics_option2 as default
  RegisterPhysics(fOpticalPhysics);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PhysicsList::~PhysicsList()
{
  delete fPhListMessenger;
  // delete fTheEMPhysics; it's managed by G4VModularPhysicsList and it can't be changed in an illegal state
  // delete fOpticalPhysics; it's managed by G4VModularPhysicsList and it can't be changed in an illegal state
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//void PhysicsList::ConstructProcess()
//{
//	
//}

//void PhysicsList::ConstructParticle()
//{
//
//}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::SetCuts()
{
  SetCutsWithDefault();
  // G4Region* worldRegion = G4RegionStore::GetInstance()->GetRegion("DefaultRegionForTheWorld");
  G4Region* radiatorRegion = G4RegionStore::GetInstance()->GetRegion("radiatorRegion");
  if (auto* theCuts = radiatorRegion->GetProductionCuts(); theCuts) {
    theCuts->SetProductionCut(fRadiatorRangeCuts_gamma, 0); /*gamma*/
    theCuts->SetProductionCut(fRadiatorRangeCuts_electron, 1); /*e-*/
    theCuts->SetProductionCut(fRadiatorRangeCuts_positron, 2); /*e+*/
    theCuts->SetProductionCut(fRadiatorRangeCuts_proton, 3); /*proton*/
  }
  else { // else should always be executed, but just in case...
    theCuts = new G4ProductionCuts{};
    theCuts->SetProductionCut(fRadiatorRangeCuts_gamma, 0); /*gamma*/
    theCuts->SetProductionCut(fRadiatorRangeCuts_electron, 1); /*e-*/
    theCuts->SetProductionCut(fRadiatorRangeCuts_positron, 2); /*e+*/
    theCuts->SetProductionCut(fRadiatorRangeCuts_proton, 3); /*proton*/
    radiatorRegion->SetProductionCuts(theCuts);
  }

  if (verboseLevel > 0)
    DumpCutValuesTable();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::ChangeEMPhysics(const UseElectromagnetic newEMPhysics)
{
  if (G4StateManager::GetStateManager()->GetCurrentState() != G4State_PreInit) {
    const char* err = "You can change registered physics only during the PreInit phase - before running G4RunManager::Initialize()\n";
    G4Exception("PhysicsList::ChangeEMPhysics", "FE_PhysList01", FatalException, err);
    return;
  }
  if (fEMPhysics == newEMPhysics) {
    std::ostringstream err;
    err << "Electromagnetic physics \"" << fEMPhysics << "\" was already loaded!\n"
      << "EM physics has not changed!\n";
    G4Exception("PhysicsList::ChangeEMPhysics", "WE_PhysList01", JustWarning, err);
    return;
  }
  if (verboseLevel > 0)
    G4cout << "Changing electromagnetic physics from " << fEMPhysics << " to " << newEMPhysics << "!\n";

  RemovePhysics(fTheEMPhysics);
  delete fTheEMPhysics; // RemovePhysics doesn't delete the physics

  switch (newEMPhysics) {
  case ChR::UseElectromagnetic::G4EmStandardPhysics:
    fTheEMPhysics = new G4EmStandardPhysics{ verboseLevel };
    RegisterPhysics(fTheEMPhysics);
    fEMPhysics = UseElectromagnetic::G4EmStandardPhysics;
    break;
  case ChR::UseElectromagnetic::G4EmStandardPhysics_option1:
    fTheEMPhysics = new G4EmStandardPhysics_option1{ verboseLevel };
    RegisterPhysics(fTheEMPhysics);
    fEMPhysics = UseElectromagnetic::G4EmStandardPhysics_option1;
    break;
  case ChR::UseElectromagnetic::G4EmStandardPhysics_option2:
    fTheEMPhysics = new G4EmStandardPhysics_option2{ verboseLevel };
    RegisterPhysics(fTheEMPhysics);
    fEMPhysics = UseElectromagnetic::G4EmStandardPhysics_option2;
    break;
  case ChR::UseElectromagnetic::G4EmStandardPhysics_option3:
    fTheEMPhysics = new G4EmStandardPhysics_option3{ verboseLevel };
    RegisterPhysics(fTheEMPhysics);
    fEMPhysics = UseElectromagnetic::G4EmStandardPhysics_option3;
    break;
  case ChR::UseElectromagnetic::G4EmStandardPhysics_option4:
    fTheEMPhysics = new G4EmStandardPhysics_option4{ verboseLevel };
    RegisterPhysics(fTheEMPhysics);
    fEMPhysics = UseElectromagnetic::G4EmStandardPhysics_option4;
    break;
  case ChR::UseElectromagnetic::G4EmLivermorePhysics:
    fTheEMPhysics = new G4EmLivermorePhysics{ verboseLevel };
    RegisterPhysics(fTheEMPhysics);
    fEMPhysics = UseElectromagnetic::G4EmLivermorePhysics;
    break;
  case ChR::UseElectromagnetic::G4EmLowEPPhysics:
    fTheEMPhysics = new G4EmLowEPPhysics{ verboseLevel };
    RegisterPhysics(fTheEMPhysics);
    fEMPhysics = UseElectromagnetic::G4EmLowEPPhysics;
    break;
  case ChR::UseElectromagnetic::G4EmPenelopePhysics:
    fTheEMPhysics = new G4EmPenelopePhysics{ verboseLevel };
    RegisterPhysics(fTheEMPhysics);
    fEMPhysics = UseElectromagnetic::G4EmPenelopePhysics;
    break;
  case ChR::UseElectromagnetic::G4EmStandardPhysicsGS:
    fTheEMPhysics = new G4EmStandardPhysicsGS{ verboseLevel };
    RegisterPhysics(fTheEMPhysics);
    fEMPhysics = UseElectromagnetic::G4EmStandardPhysicsGS;
    break;
  case ChR::UseElectromagnetic::G4EmStandardPhysicsSS:
    fTheEMPhysics = new G4EmStandardPhysicsSS{ verboseLevel };
    RegisterPhysics(fTheEMPhysics);
    fEMPhysics = UseElectromagnetic::G4EmStandardPhysicsSS;
    break;
  case ChR::UseElectromagnetic::G4EmStandardPhysicsWVI:
    fTheEMPhysics = new G4EmStandardPhysicsWVI{ verboseLevel };
    RegisterPhysics(fTheEMPhysics);
    fEMPhysics = UseElectromagnetic::G4EmStandardPhysicsWVI;
    break;
  default:
    G4Exception("PhysicsList::ChangeEMPhysics", "FE_PhysList02", FatalException, "This one should not have happened\n");
    break;
  }

  if (verboseLevel > 0)
    G4cout << "New electromagnetic physics " << fEMPhysics << " has been registered!\n";
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::ChangeOpticalPhysics(const UseOptical newOpticalPhysics)
{
  if (G4StateManager::GetStateManager()->GetCurrentState() != G4State_PreInit) {
    const char* err = "You can change registered physics only during the PreInit phase - before running G4RunManager::Initialize()\n";
    G4Exception("PhysicsList::ChangeOpticalPhysics", "FE_PhysList03", FatalException, err);
    return;
  }
  if (fOptical == newOpticalPhysics) {
    std::ostringstream err;
    err << "Optical physics \"" << fOptical << "\" was already loaded!\n"
      << "Optical physics has not changed!\n";
    G4Exception("PhysicsList::ChangeOpticalPhysics", "WE_PhysList02", JustWarning, err);
    return;
  }

  if (verboseLevel > 0)
    G4cout << "Changing electromagnetic physics from " << fOptical << " to " << newOpticalPhysics << "!\n";

  RemovePhysics(fOpticalPhysics);
  delete fOpticalPhysics; // RemovePhysics doesn't delete the physics

  switch (newOpticalPhysics) {
  case ChR::UseOptical::G4OpticalPhysics:
    fOpticalPhysics = new G4OpticalPhysics{ verboseLevel };
    RegisterPhysics(fOpticalPhysics);
    fOptical = UseOptical::G4OpticalPhysics;
    break;
  case ChR::UseOptical::G4OpticalPhysics_option1:
    fOpticalPhysics = new G4OpticalPhysics_option1{ verboseLevel };
    RegisterPhysics(fOpticalPhysics);
    fOptical = UseOptical::G4OpticalPhysics_option1;
    break;
  case ChR::UseOptical::G4OpticalPhysics_option2:
    fOpticalPhysics = new G4OpticalPhysics_option2{ verboseLevel };
    RegisterPhysics(fOpticalPhysics);
    fOptical = UseOptical::G4OpticalPhysics_option2;
    break;
  default: // how??
    G4Exception("PhysicsList::ChangeOpticalPhysics", "FE_PhysList04", FatalException, "This one should not have happened\n");
    break;
  }

  if (verboseLevel > 0)
    G4cout << "New electromagnetic physics " << fOptical << " has been registered!\n";
}

endChR