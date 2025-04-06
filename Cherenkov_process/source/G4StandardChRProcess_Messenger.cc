//##########################################
//#######        VERSION 1.1.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#include "G4StandardChRProcess_Messenger.hh"
#include "G4StandardCherenkovProcess.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4MTRunManager.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo...... 
//=========public G4StandardChRProcess_Messenger:: methods=========
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo...... 

G4StandardChRProcess_Messenger::G4StandardChRProcess_Messenger(G4StandardCherenkovProcess* theChRProcess)
: fStandardChRProcess(theChRProcess),
  fChRProcessDir(new G4UIdirectory{ "/process/optical/stdChRProcess/" }),
  fDumpChRInfo(new G4UIcommand{ "/process/optical/stdChRProcess/dumpInfo", this }),
  fProcessDescription(new G4UIcommand{ "/process/optical/stdChRProcess/processDescription", this }),
  fIsApplicable(new G4UIcmdWithAString{ "/process/optical/stdChRProcess/isApplicable", this }),
  fMinEnergy(new G4UIcommand{ "/process/optical/stdChRProcess/minEnergy", this }),
  fNoOfBetaSteps(new G4UIcmdWithAnInteger{ "/process/optical/stdChRProcess/noOfBetaSteps", this }),
  fUseEnergyLoss(new G4UIcmdWithABool{ "/process/optical/stdChRProcess/useEnergyLoss", this }),
  fPrintPhysicsVector(new G4UIcommand{ "/process/optical/stdChRProcess/printChRPhysicsVector", this }),
  fExoticRIndex(new G4UIcommand{ "/process/optical/stdChRProcess/changeUseOfExoticRIndex", this })
{
  //DIR
  fChRProcessDir->SetGuidance("All commands related to G4StandardCherenkovProcess");
  //commands
  fDumpChRInfo->SetGuidance("Used to print information about the G4StandardCherenkovProcess class.");
  fDumpChRInfo->SetToBeBroadcasted(false);
  fDumpChRInfo->AvailableForStates(G4State_Idle, G4State_GeomClosed, G4State_EventProc);

  fProcessDescription->SetGuidance("Used to print about what Cherenkov process is.");
  fProcessDescription->SetToBeBroadcasted(false);
  fProcessDescription->AvailableForStates(G4State_Idle, G4State_GeomClosed, G4State_EventProc);

  fIsApplicable->SetGuidance("Check if a specific particle can generate Cherenkov photons.");
  fIsApplicable->SetGuidance("NOTE: Make sure to write a particle name correctly.");
  fIsApplicable->SetParameterName("particleName", false);
  fIsApplicable->SetToBeBroadcasted(false);
  fIsApplicable->AvailableForStates(G4State_Idle, G4State_GeomClosed, G4State_EventProc);

  fMinEnergy->SetGuidance("Check if a specific particle can generate Cherenkov photons");
  fMinEnergy->SetGuidance("for a specific material, and print the minimal energy");
  fMinEnergy->SetGuidance("needed for Cherenkov photons' generation.");
  fMinEnergy->SetGuidance("You need to provide two strings:");
  fMinEnergy->SetGuidance("1. A name of the particle");
  fMinEnergy->SetGuidance("2. A name of the registered material");
  //G4UIParameter objects are deleted in deconstructor of G4UIcommand
  G4UIparameter* uiParameter = new G4UIparameter{ "particleName", 's', false };
  fMinEnergy->SetParameter(uiParameter);
  uiParameter = new G4UIparameter{ "materialName", 's', false };
  fMinEnergy->SetParameter(uiParameter);
  fMinEnergy->SetToBeBroadcasted(false);
  fMinEnergy->AvailableForStates(G4State_Idle, G4State_GeomClosed, G4State_EventProc);

  fUseEnergyLoss->SetGuidance("Used to activate energy conservation law for Cherenkov process.");
  fUseEnergyLoss->SetGuidance("According to G4Cerenkov and Frank-Tamm theory, Cherenkov radiation doesn't change charged particle's energy.");
  fUseEnergyLoss->SetGuidance("However, if you select \"true\" in new models, the energy change is considered (note that it's minimal anyway).");
  fUseEnergyLoss->SetParameterName("changeEnergyInChR", true);
  fUseEnergyLoss->SetDefaultValue(true);
  fUseEnergyLoss->SetToBeBroadcasted(true);
  fUseEnergyLoss->AvailableForStates(G4State_Idle);

  fNoOfBetaSteps->SetGuidance("Used to change the number of beta steps (betaNodes == betaStep + 1).");
  fNoOfBetaSteps->SetGuidance("When building physics tables for G4StandardChRProcess, the critical energies are considered through the relativistic velocity \"beta\" of the charged particle.");
  fNoOfBetaSteps->SetGuidance("For critical energies, the physics tables are divided into steps from minimal beta to maximal beta.");
  fNoOfBetaSteps->SetParameterName("betaSteps", false);
  fNoOfBetaSteps->SetRange("betaSteps>=1 && betaSteps<=255");
  fNoOfBetaSteps->SetToBeBroadcasted(false);
  fNoOfBetaSteps->AvailableForStates(G4State_Idle);

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
  fPrintPhysicsVector->AvailableForStates(G4State_Idle, G4State_GeomClosed, G4State_EventProc);

  fExoticRIndex->SetGuidance("Used to change whether exotic refractive indices will be used for a");
  fExoticRIndex->SetGuidance("specific material or not.");
  fExoticRIndex->SetGuidance("IF YOU CHANGE THE NUMBER OF BETA STEPS BY COMMAND:");
  fExoticRIndex->SetGuidance("\"/process/optical/stdChRProcess/noOfBetaSteps\"");
  fExoticRIndex->SetGuidance("AFTER USING THIS COMMAND, YOU WILL LOSE THE RESULT OF THIS COMMAND!");
  uiParameter = new G4UIparameter{ "materialName", 's', false };
  fExoticRIndex->SetParameter(uiParameter);
  uiParameter = new G4UIparameter{ "exoticRIndex", 'b', false };
  fExoticRIndex->SetParameter(uiParameter);
  fExoticRIndex->SetToBeBroadcasted(false);
  fExoticRIndex->AvailableForStates(G4State_Idle);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo...... 

G4StandardChRProcess_Messenger::~G4StandardChRProcess_Messenger()
{
  //DIR
  delete fChRProcessDir;
  //ChR commands
  delete fDumpChRInfo;
  delete fProcessDescription;
  delete fIsApplicable;
  delete fMinEnergy;
  delete fNoOfBetaSteps;
  delete fUseEnergyLoss;
  delete fPrintPhysicsVector;
  delete fExoticRIndex;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo...... 

void G4StandardChRProcess_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr)
{
  if (uiCmd == fDumpChRInfo) {
    fStandardChRProcess->DumpInfo();
  }
  else if (uiCmd == fProcessDescription) {
    fStandardChRProcess->ProcessDescription();
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
      if (fStandardChRProcess->IsApplicable(*(itr->value())))
        std::cout << "Particle " << aStr << " can generate Cherenkov photons!\n";
      else
        std::cout << "Particle " << aStr << " cannot generate Cherenkov photons!\n";
    }
    else {
      std::ostringstream err;
      err << "There is no registered particle under the name " << std::quoted(aStr) << '\n';
      G4Exception("G4StandardChRProcess_Messenger::SetNewValue", "WE_stdChRMessenger01", JustWarning, err);
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
      G4Exception("G4StandardChRProcess_Messenger::SetNewValue", "WE_stdChRMessenger02", JustWarning, err);
      return;
    }
    if (!fStandardChRProcess->IsApplicable(*itr->value())) {
      std::cout
        << "A particle " << std::quoted(itr->value()->GetParticleName())
        << " cannot generate Cherenkov photons in general!\n" << std::endl;
      return;
    }
    G4Material* theMaterial = G4Material::GetMaterial(materialName);
    if (!theMaterial) {
      std::ostringstream err;
      err << "There is no registered material under the name " << std::quoted(materialName) << '\n';
      G4Exception("G4StandardChRProcess_Messenger::SetNewValue", "WE_stdChRMessenger03", JustWarning, err);
      return;
    }
    G4double minEnergy = fStandardChRProcess->MinPrimaryEnergy(itr->value(), theMaterial);
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
  else if (uiCmd == fNoOfBetaSteps) {
    const unsigned int newBetaStep = std::stoul(aStr);
    if (newBetaStep == fStandardChRProcess->GetNoOfBetaSteps()) {
      const char* msg = "betaStep of Cherenkov models has not been changed - you used the same number that's already set!\n";
      G4Exception("G4StandardChRProcess_Messenger::SetNewValue", "WE_stdChRMessenger04", JustWarning, msg);
      return;
    }
    fStandardChRProcess->SetNoOfBetaSteps(newBetaStep);
    if (fStandardChRProcess->verboseLevel > 0)
      std::cout << "All betaStep values have been changed! Now deleting old physics tables...\n";
    fStandardChRProcess->fChRPhysDataVec.clear();
    if (fStandardChRProcess->verboseLevel > 0)
      std::cout << "Old physics tables have been removed! Now creating new physics tables...\n";
    auto* particleIterator = G4ParticleTable::GetParticleTable()->GetIterator();
    particleIterator->reset();
    while ((*particleIterator)()) { //a useless and bad loop for this process, but still... pre-run time
      fStandardChRProcess->BuildPhysicsTable(*(particleIterator->value()));
    }
    if (fStandardChRProcess->verboseLevel > 0)
      std::cout << "Physics tables have been successfully rebuilt!\n";
  }
  else if (uiCmd == fUseEnergyLoss) {
    fStandardChRProcess->SetUseEnergyLoss(fUseEnergyLoss->ConvertToBool(aStr));
  }
  else if (uiCmd == fPrintPhysicsVector) {
    std::string printLevel, materialName;
    // aStr returns a space (' ') character for nothing??
    if (aStr == G4String{ ' ' }) {
      fStandardChRProcess->PrintChRPhysDataVec();
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
        G4Exception("G4StandardChRProcess_Messenger::SetNewValue", "WE_stdChRMessenger05", JustWarning, err);
        return;
      }
      fStandardChRProcess->PrintChRPhysDataVec(printLevelNumber, aMaterial);
      return;
    }
    fStandardChRProcess->PrintChRPhysDataVec(printLevelNumber);
  }
  else if (uiCmd == fExoticRIndex) {
    std::string matName, newFlag;
    std::string::iterator spaceChar1 = std::find(aStr.begin(), aStr.end(), ' ');
    std::copy(aStr.begin(), spaceChar1, std::back_inserter(matName));
    std::string::iterator spaceChar2 = std::find(spaceChar1 + 1, aStr.end(), ' ');
    std::copy(spaceChar1 + 1, spaceChar2, std::back_inserter(newFlag));
    G4Material* aMaterial = G4Material::GetMaterial(matName);
    if (!aMaterial) {
      std::ostringstream err;
      err << "You wrote that the name of a material is: " << std::quoted(matName)
        << "\nwhile there's no such a material. Please, check the names again!\n";
      G4Exception("G4ExtraOpticalParameters_Messenger::SetNewValue", "WE_stdChRMessenger06", JustWarning, err);
      return;
    }
    G4bool tableExists;
    if (fStandardChRProcess->fChRPhysDataVec[aMaterial->GetIndex()].fAroundBetaValues.front().fValuesCDF)
      tableExists = true;
    else
      tableExists = false;
    G4bool newValue = fExoticRIndex->ConvertToBool(newFlag.c_str());
    if (newValue == tableExists)
      return;
    if (newValue) {
      if (!fStandardChRProcess->AddExoticRIndexPhysicsTable(aMaterial->GetIndex(), true)) {
        std::ostringstream err;
        err << "Exotic RIndex tables are not built for material " << std::quoted(matName)
          << "\nThe material's RIndex is not suitable for exotic RIndex tables!\n";
        G4Exception("G4ExtraOpticalParameters_Messenger::SetNewValue", "WE_stdChRMessenger07", JustWarning, err);
      }
    }
    else
      fStandardChRProcess->RemoveExoticRIndexPhysicsTable(aMaterial->GetIndex());
  }
  else { //just in case of some bug, but it can be removed
    G4Exception("G4StandardChRProcess_Messenger::SetNewValue", "WE_stdChRMessenger08", JustWarning, "Command not found!\n");
  }
}