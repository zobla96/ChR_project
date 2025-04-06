//##########################################
//#######        VERSION 1.1.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

// user headers
#include "PhysicsList_Messenger.hh"
// G4 headers
#include "G4UIdirectory.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"

beginChR

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//=========public ChR::PhysicsList_Messenger:: methods=========
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PhysicsList_Messenger::PhysicsList_Messenger(PhysicsList* phList)
: fPhysicsList(phList),
  fPhysListDir(new G4UIdirectory{ "/ChR_project/PhysicsList/" }),
  fGammaRangeCut(new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PhysicsList/gammaRangeCut", this }),
  fElectronRangeCut(new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PhysicsList/electronRangeCut", this }),
  fPositronRangeCut(new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PhysicsList/positronRangeCut", this }),
  fProtonRangeCut(new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PhysicsList/protonRangeCut", this }),
  fSelectEmPhysics(new G4UIcmdWithAnInteger{ "/ChR_project/PhysicsList/selectEmPhysics", this }),
  fSelectOpticalPhysics(new G4UIcmdWithAnInteger{ "/ChR_project/PhysicsList/selectOpticalPhysics", this })
{

  fPhysListDir->SetGuidance("All commands related to ChR::PhysicsList");

  fGammaRangeCut->SetGuidance("Used to change the gamma-cut value.");
  fGammaRangeCut->SetParameterName("gammaCut", false);
  fGammaRangeCut->SetDefaultUnit("um");
  fGammaRangeCut->SetToBeBroadcasted(false);
  fGammaRangeCut->AvailableForStates(G4State_PreInit);

  fElectronRangeCut->SetGuidance("Used to change the electron-cut value.");
  fElectronRangeCut->SetParameterName("electronCut", false);
  fElectronRangeCut->SetDefaultUnit("um");
  fElectronRangeCut->SetToBeBroadcasted(false);
  fElectronRangeCut->AvailableForStates(G4State_PreInit);

  fPositronRangeCut->SetGuidance("Used to change the positron-cut value.");
  fPositronRangeCut->SetParameterName("positronCut", false);
  fPositronRangeCut->SetDefaultUnit("um");
  fPositronRangeCut->SetToBeBroadcasted(false);
  fPositronRangeCut->AvailableForStates(G4State_PreInit);

  fProtonRangeCut->SetGuidance("Used to change the proton-cut value.");
  fProtonRangeCut->SetParameterName("protonCut", false);
  fProtonRangeCut->SetDefaultUnit("um");
  fProtonRangeCut->SetToBeBroadcasted(false);
  fProtonRangeCut->AvailableForStates(G4State_PreInit);

  fSelectEmPhysics->SetGuidance("Used to change the EM physics that will be used.");
  fSelectEmPhysics->SetGuidance("There are various EM physics options, select a number of the desired EM physics from the list:");
  fSelectEmPhysics->SetGuidance("0 - G4EmStandardPhysics\n1 - G4EmStandardPhysics_option1\n2 - G4EmStandardPhysics_option2");
  fSelectEmPhysics->SetGuidance("3 - G4EmStandardPhysics_option3\n4 - G4EmStandardPhysics_option4\n5 - G4EmLivermorePhysics");
  fSelectEmPhysics->SetGuidance("6 - G4EmLowEPPhysics\n7 - G4EmPenelopePhysics\n8 - G4EmStandardPhysicsGS");
  fSelectEmPhysics->SetGuidance("9 - G4EmStandardPhysicsSS\n10 - G4EmStandardPhysicsWVI");
  fSelectEmPhysics->SetParameterName("EmPhysicsNoFromTheList", false);
  fSelectEmPhysics->SetRange("EmPhysicsNoFromTheList>=0 && EmPhysicsNoFromTheList<11");
  fSelectEmPhysics->SetToBeBroadcasted(false);
  fSelectEmPhysics->AvailableForStates(G4State_PreInit);

  fSelectOpticalPhysics->SetGuidance("Used to change optical physics into:");
  fSelectOpticalPhysics->SetGuidance("0 - G4OpticalPhysics\n1 - G4OpticalPhysics_option1\n2 - G4OpticalPhysics_option2");
  fSelectOpticalPhysics->SetParameterName("opticalPhysID", false);
  fSelectOpticalPhysics->SetRange("opticalPhysID>=0 && opticalPhysID<3");
  fSelectOpticalPhysics->SetToBeBroadcasted(false);
  fSelectOpticalPhysics->AvailableForStates(G4State_PreInit);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PhysicsList_Messenger::~PhysicsList_Messenger()
{
  delete fPhysListDir;
  delete fGammaRangeCut;
  delete fElectronRangeCut;
  delete fPositronRangeCut;
  delete fProtonRangeCut;
  delete fSelectEmPhysics;
  delete fSelectOpticalPhysics;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr)
{
  if (uiCmd == fGammaRangeCut) {
    fPhysicsList->SetRadiatorRangeCuts_gamma(fGammaRangeCut->GetNewDoubleValue(aStr));
  }
  else if (uiCmd == fElectronRangeCut) {
    fPhysicsList->SetRadiatorRangeCuts_electron(fElectronRangeCut->GetNewDoubleValue(aStr));
  }
  else if (uiCmd == fPositronRangeCut) {
    fPhysicsList->SetRadiatorRangeCuts_positron(fPositronRangeCut->GetNewDoubleValue(aStr));
  }
  else if (uiCmd == fProtonRangeCut) {
    fPhysicsList->SetRadiatorRangeCuts_proton(fPositronRangeCut->GetNewDoubleValue(aStr));
  }
  else if (uiCmd == fSelectEmPhysics) {
    G4int emIdNo = fSelectEmPhysics->ConvertToInt(aStr);
    switch (emIdNo) {
    case 0:
      fPhysicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysics);
      break;
    case 1:
      fPhysicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysics_option1);
      break;
    case 2:
      fPhysicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysics_option2);
      break;
    case 3:
      fPhysicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysics_option3);
      break;
    case 4:
      fPhysicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysics_option4);
      break;
    case 5:
      fPhysicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmLivermorePhysics);
      break;
    case 6:
      fPhysicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmLowEPPhysics);
      break;
    case 7:
      fPhysicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmPenelopePhysics);
      break;
    case 8:
      fPhysicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysicsGS);
      break;
    case 9:
      fPhysicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysicsSS);
      break;
    case 10:
      fPhysicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysicsWVI);
      break;
    default: // shouldn't happen
      G4Exception("PhysicsList_Messenger::SetNewValue", "FE_PhysListMessenger01", FatalException, "Somehow, selected EM physics not found!\n");
      break;
    }
  }
  else if (uiCmd == fSelectOpticalPhysics) {
    G4int opticalID = fSelectOpticalPhysics->ConvertToInt(aStr);
    switch (opticalID) {
    case 0:
      fPhysicsList->ChangeOpticalPhysics(UseOptical::G4OpticalPhysics);
      break;
    case 1:
      fPhysicsList->ChangeOpticalPhysics(UseOptical::G4OpticalPhysics_option1);
      break;
    case 2:
      fPhysicsList->ChangeOpticalPhysics(UseOptical::G4OpticalPhysics_option2);
      break;
    default: // shouldn't happen
      G4Exception("PhysicsList_Messenger::SetNewValue", "FE_PhysListMessenger02", FatalException, "Somehow, selected optical physics not found!\n");
      break;
    }
  }
  else
    G4Exception("PhysicsList_Messenger::SetNewValue", "FE_PhysListMessenger03", FatalException, "Command not found!\n");
}

endChR