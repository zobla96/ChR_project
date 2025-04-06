//##########################################
//#######        VERSION 1.1.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//G4 headers
#include "G4CherenkovProcess_Messenger.hh"
#include "G4CherenkovProcess.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIparameter.hh"
#include "G4ParticleTable.hh"
#include "G4MTRunManager.hh"
#include "G4SystemOfUnits.hh"
//std:: headers
#include "sstream"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//=========public G4CherenkovProcess_Messenger:: methods=========
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4CherenkovProcess_Messenger::G4CherenkovProcess_Messenger(G4CherenkovProcess* theChRProcess)
: fChRProcess(theChRProcess),
  fChRMessengerDir(new G4UIdirectory{ "/process/optical/G4ChRProcess/" }),
  fBaseChRModelUIDirectory(new G4UIdirectory{ "/process/optical/G4ChRProcess/Models/" }),
  fDumpChRInfo(new G4UIcommand{ "/process/optical/G4ChRProcess/dumpInfo", this }),
  fProcessDescription(new G4UIcommand{ "/process/optical/G4ChRProcess/processDescription", this }),
  fIsApplicable(new G4UIcmdWithAString{ "/process/optical/G4ChRProcess/isApplicable", this }),
  fMinEnergy(new G4UIcommand{ "/process/optical/G4ChRProcess/minEnergy",this }),
  fUseEnergyLossInModels(new G4UIcmdWithABool{ "/process/optical/G4ChRProcess/Models/useEnergyLossInModels", this }),
  fNoOfBetaSteps(new G4UIcmdWithAnInteger{ "/process/optical/G4ChRProcess/Models/noOfBetaSteps", this }),
  fModelVerboseLevel(new G4UIcmdWithAnInteger{ "/process/optical/G4ChRProcess/Models/changeModelVerbose", this }),
  fPrintPhysicsVector(new G4UIcommand{ "/process/optical/G4ChRProcess/Models/printBaseChRPhysicsVector", this })
{

  G4UIparameter* uiParameter = nullptr;

  //DIR #1
  fChRMessengerDir->SetGuidance("All commands related to G4CherenkovProcess");
  //ChR commands
  fDumpChRInfo->SetGuidance("Used to print all available information about Cherenkov process class and registered models.");
  fDumpChRInfo->SetToBeBroadcasted(false);
  fDumpChRInfo->AvailableForStates(G4State_Idle);

  fProcessDescription->SetGuidance("Used to print about what Cherenkov process is.");
  fProcessDescription->SetToBeBroadcasted(false);
  fProcessDescription->AvailableForStates(G4State_Idle);

  fIsApplicable->SetGuidance("Check if a specific particle can generate Cherenkov photons.");
  fIsApplicable->SetGuidance("NOTE: Make sure to write a particle name correctly.");
  fIsApplicable->SetParameterName("particleName", false);
  fIsApplicable->SetToBeBroadcasted(false);
  fIsApplicable->AvailableForStates(G4State_Idle);

  fMinEnergy->SetGuidance("Check if a specific particle can generate Cherenkov photons");
  fMinEnergy->SetGuidance("for a specific material, and print the minimal energy");
  fMinEnergy->SetGuidance("needed for Cherenkov photons' generation.");
  fMinEnergy->SetGuidance("You need to provide two strings:");
  fMinEnergy->SetGuidance("1. A name of the particle");
  fMinEnergy->SetGuidance("2. A name of the registered material");
  //G4UIParameter objects are deleted in deconstructor of G4UIcommand
  uiParameter = new G4UIparameter{ "particleName", 's', false };
  fMinEnergy->SetParameter(uiParameter);
  uiParameter = new G4UIparameter{ "materialName", 's', false };
  fMinEnergy->SetParameter(uiParameter);
  fMinEnergy->SetToBeBroadcasted(false);
  fMinEnergy->AvailableForStates(G4State_Idle);

  //DIR #2
  fBaseChRModelUIDirectory->SetGuidance("All commands related to registered models in G4CherenkovProcess");
  //BaseChR_Model commands
  fUseEnergyLossInModels->SetGuidance("Used to activate energy conservation law for all registered models.");
  fUseEnergyLossInModels->SetGuidance("According to G4Cerenkov and Frank-Tamm theory, Cherenkov radiation doesn't change charged particle's energy.");
  fUseEnergyLossInModels->SetGuidance("However, if you select \"true\" in new models, the energy change is considered (note that it's minimal anyway).");
  fUseEnergyLossInModels->SetParameterName("changeEnergyInChR", true);
  fUseEnergyLossInModels->SetDefaultValue(true);
  fUseEnergyLossInModels->SetToBeBroadcasted(true);
  fUseEnergyLossInModels->AvailableForStates(G4State_Idle);

  fNoOfBetaSteps->SetGuidance("Used to change the number of beta steps (betaNodes == betaSteps + 1).");
  fNoOfBetaSteps->SetGuidance("When building physics tables for BaseChR_Model, the critical energies are considered through the relativistic velocity \"beta\" of the charged particle.");
  fNoOfBetaSteps->SetGuidance("For critical energies, the physics tables are divided into steps from minimal beta to maximal beta.");
  fNoOfBetaSteps->SetParameterName("betaSteps", false);
  fNoOfBetaSteps->SetRange("betaSteps>=1");
  fNoOfBetaSteps->SetToBeBroadcasted(false);
  fNoOfBetaSteps->AvailableForStates(G4State_Idle);

  fModelVerboseLevel->SetGuidance("Used to change the verbose level for all registered models.");
  fModelVerboseLevel->SetParameterName("verboseLevel", true);
  fModelVerboseLevel->SetDefaultValue(1);
  fModelVerboseLevel->SetRange("verboseLevel>=0 && verboseLevel<256");
  fModelVerboseLevel->SetToBeBroadcasted(true);
  fModelVerboseLevel->AvailableForStates(G4State_Idle);

  fPrintPhysicsVector->SetGuidance("Used to print the loaded static physics vector of BaseChR_model.");
  fPrintPhysicsVector->SetGuidance("Used to print the loaded static physics vector of G4StandardChRProcess.");
  fPrintPhysicsVector->SetGuidance("printLevel == 0 -> print only basic available information about registered physics tables");
  fPrintPhysicsVector->SetGuidance("printLevel == 1 -> print standard + exotic RIndex CDF values");
  fPrintPhysicsVector->SetGuidance("printLevel >= 2 -> print all available information about registered physics tables");
  fPrintPhysicsVector->SetGuidance("materialName - omitted -> prints physics tables for all registered materials");
  fPrintPhysicsVector->SetGuidance("materialName - provided -> prints physics tables for materialName");
  uiParameter = new G4UIparameter{ "printLevel", 'i', true };
  uiParameter->SetParameterRange("printLevel>=0 && printLevel<256");
  fPrintPhysicsVector->SetParameter(uiParameter);
  uiParameter = new G4UIparameter{ "materialName", 's', true };
  fPrintPhysicsVector->SetParameter(uiParameter);
  fPrintPhysicsVector->SetToBeBroadcasted(false);
  fPrintPhysicsVector->AvailableForStates(G4State_Idle);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4CherenkovProcess_Messenger::~G4CherenkovProcess_Messenger()
{
  //2x DIR
  delete fChRMessengerDir;
  delete fBaseChRModelUIDirectory;
  //G4CherenkovProcess commands
  delete fDumpChRInfo;
  delete fProcessDescription;
  delete fIsApplicable;
  delete fMinEnergy;
  //G4BaseChR_Model commands
  delete fUseEnergyLossInModels;
  delete fNoOfBetaSteps;
  delete fModelVerboseLevel;
  delete fPrintPhysicsVector;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void G4CherenkovProcess_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr)
{
  if (uiCmd == fDumpChRInfo) {
    fChRProcess->DumpInfo();
  }
  else if (uiCmd == fProcessDescription) {
    fChRProcess->ProcessDescription();
  }
  else if (uiCmd == fIsApplicable) {
    G4ParticleTable::G4PTblDicIterator* itr = G4ParticleTable::GetParticleTable()->GetIterator();
    itr->reset();
    G4bool foundIt = false;
    while ((*itr)()) {
      if (itr->value()->GetParticleName() == aStr) {
        foundIt = true;
        break;
      }
    }
    if (foundIt) {
      if (fChRProcess->IsApplicable(*(itr->value())))
        std::cout << "Particle " << aStr << " can generate Cherenkov photons!\n";
      else
        std::cout << "Particle " << aStr << " cannot generate Cherenkov photons!\n";
    }
    else {
      std::ostringstream err;
      err << "There is no registered particle under the name " << std::quoted(aStr) << '\n';
      G4Exception("G4CherenkovProcess_Messenger::SetNewValue", "WE_ChRMessenger01", JustWarning, err);
    }
  }
  else if (uiCmd == fMinEnergy) {
    std::string particleName, materialName;
    std::string::iterator spaceChar = std::find(aStr.begin(), aStr.end(), ' ');
    std::copy(aStr.begin(), spaceChar, std::back_inserter(particleName));
    std::copy(spaceChar + 1, aStr.end(), std::back_inserter(materialName));
    G4ParticleTable::G4PTblDicIterator* itr = G4ParticleTable::GetParticleTable()->GetIterator();
    itr->reset();
    G4bool foundIt = false;
    while ((*itr)()) {
      if (itr->value()->GetParticleName() == particleName) {
        foundIt = true;
        break;
      }
    }
    if (!foundIt) {
      std::ostringstream err;
      err << "There is no registered particle under the name " << std::quoted(particleName) << '\n';
      G4Exception("G4CherenkovProcess_Messenger::SetNewValue", "WE_ChRMessenger02", JustWarning, err);
      return;
    }
    if (!fChRProcess->IsApplicable(*itr->value())) {
      std::cout
        << "A particle " << std::quoted(itr->value()->GetParticleName())
        << " cannot generate Cherenkov photons in general!\n" << std::endl;
      return;
    }
    G4Material* theMaterial = G4Material::GetMaterial(materialName);
    if (!theMaterial) {
      std::ostringstream err;
      err << "There is no registered material under the name " << std::quoted(materialName) << '\n';
      G4Exception("G4CherenkovProcess_Messenger::SetNewValue", "WE_ChRMessenger03", JustWarning, err);
      return;
    }
    G4double minEnergy = fChRProcess->MinPrimaryEnergy(itr->value(), theMaterial);
    if (minEnergy > 0) {
      std::cout
        << "The minimal energy of a particle " << std::quoted(itr->value()->GetParticleName())
        << "\nthat is needed to generate Cherenkov photons in the registered material\n"
        << std::quoted(theMaterial->GetName()) << " is: " << minEnergy / MeV << " MeV\n" << std::endl;
    }
    else {
      std::cout
        << "A particle " << std::quoted(itr->value()->GetParticleName())
        << " cannot generate Cherenkov photons\nin the registered material "
        << std::quoted(theMaterial->GetName()) << '\n' << std::endl;
    }
  }
  else if (uiCmd == fUseEnergyLossInModels) {
    G4bool newValue = fUseEnergyLossInModels->ConvertToBool(aStr);
    for (auto* aModel : fChRProcess->fRegisteredModels)
      aModel->SetUseModelWithEnergyLoss(newValue);
  }
  else if (uiCmd == fNoOfBetaSteps) {
    const unsigned int newBetaStep = std::stoul(aStr);
    if (newBetaStep == G4BaseChR_Model::GetNoOfBetaSteps()) {
      const char* msg = "betaStep of Cherenkov models has not been changed - you used the same number that's already set!\n";
      G4Exception("G4CherenkovProcess_Messenger::SetNewValue", "WE_ChRMessenger04", JustWarning, msg);
      return;
    }
    G4BaseChR_Model::SetNoOfBetaSteps(newBetaStep);
    if ((*fChRProcess->fRegisteredModels.begin())->GetVerboseLevel() > 0)
      std::cout << "The betaStep value has been changed! Now deleting old physics tables...\n";
    const_cast<G4BaseChR_Model::G4ChRPhysicsTableVector&>(G4BaseChR_Model::GetChRPhysDataVec()).clear();
    if ((*fChRProcess->fRegisteredModels.begin())->GetVerboseLevel() > 0)
      std::cout << "Old physics tables have been removed! Now creating new physics tables...\n";
    auto* particleIterator = G4ParticleTable::GetParticleTable()->GetIterator();
    particleIterator->reset();
    while ((*particleIterator)()) { //a useless and bad loop for this process, but still... pre-run time
      (*fChRProcess->fRegisteredModels.begin())->BuildModelPhysicsTable(*(particleIterator->value()));
    }
    if ((*fChRProcess->fRegisteredModels.begin())->GetVerboseLevel() > 0)
      std::cout << "Physics tables have been successfully rebuilt!\n";
  }
  else if (uiCmd == fModelVerboseLevel) {
    const unsigned char newValue = (const unsigned char)fModelVerboseLevel->ConvertToInt(aStr);
    for (auto* aModel : fChRProcess->fRegisteredModels)
      aModel->SetVerboseLevel(newValue);
  }
  else if (uiCmd == fPrintPhysicsVector) {
    std::string printLevel, materialName;
    // aStr returns a space (' ') character for nothing??
    if (aStr == G4String{ ' ' }) {
      G4BaseChR_Model::PrintChRPhysDataVec();
      return;
    }
    std::string::iterator spaceChar = std::find(aStr.begin(), aStr.end(), ' ');
    std::copy(aStr.begin(), spaceChar, std::back_inserter(printLevel));
    const unsigned char printLevelNumber = (const unsigned char)std::stoul(printLevel);
    if (spaceChar + 1 != aStr.end()) {
      std::copy(spaceChar + 1, aStr.end(), std::back_inserter(materialName));
      G4Material* aMaterial = nullptr;
      if (!(aMaterial = G4Material::GetMaterial(materialName))) {
        std::ostringstream err;
        err << "You wrote that the name of a material is: " << std::quoted(materialName)
            << "\nwhile there's no such a material. Please, check the names again!\n"
            << "Physics tables not printed!\n";
        G4Exception("G4CherenkovProcess_Messenger::SetNewValue", "WE_ChRMessenger05", JustWarning, err);
        return;
      }
      G4BaseChR_Model::PrintChRPhysDataVec(printLevelNumber, aMaterial);
      return;
    }
    G4BaseChR_Model::PrintChRPhysDataVec(printLevelNumber);
  }
  else //just in case of some bug, but it can be removed
    G4Exception("G4CherenkovProcess_Messenger::SetNewValue", "WE_ChRMessenger06", JustWarning, "Command not found!\n");
}