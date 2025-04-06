//##########################################
//#######        VERSION 1.1.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

// user headers
#include "PrimaryGeneratorAction_Messenger.hh"
// G4 headers
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

beginChR

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//=========public ChR::PrimaryGeneratorAction_Messenger:: methods=========
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction_Messenger::PrimaryGeneratorAction_Messenger(PrimaryGeneratorAction* theGenerator)
: fPrimaryGenerator(theGenerator),
  fPGeneratorMessengerDir(new G4UIdirectory{ "/ChR_project/PrimaryGenerator/" }),
  fBeamSigma(new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PrimaryGenerator/beamSigma", this }),
  fDivSigma(new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PrimaryGenerator/thetaBeamDivergence", this })
{

  fPGeneratorMessengerDir->SetGuidance("All commands related to primary generator action");

  fBeamSigma->SetGuidance("Used to change the Gaussian sigma of the beam.");
  fBeamSigma->SetGuidance("If this value is set to '0.', pencil-like beams are used. Otherwise, more realistic Gaussian beams are used.");
  fBeamSigma->SetGuidance("Gaussian distribution and this parameter are implemented as std::normal_distribution");
  fBeamSigma->SetParameterName("GaussSigma", false);
  fBeamSigma->SetDefaultUnit("um");
  fBeamSigma->SetRange("GaussSigma>=0.");
  fBeamSigma->SetToBeBroadcasted(true);
  fBeamSigma->AvailableForStates(G4State_Idle); //keeping it hidden from the early master (there's no such an instance there)

  fDivSigma->SetGuidance("Used to change the divergence angle as normal distribution.");
  fDivSigma->SetGuidance("It is used to simulate beam divergence as a normal distribution with the given sigma of the angle theta.");
  fDivSigma->SetParameterName("thetaBeamDivergence", false);
  fDivSigma->SetDefaultUnit("mrad");
  fDivSigma->SetRange("thetaBeamDivergence>=0.");
  fDivSigma->SetToBeBroadcasted(true);
  fDivSigma->AvailableForStates(G4State_Idle);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction_Messenger::~PrimaryGeneratorAction_Messenger()
{
  delete fPGeneratorMessengerDir;
  delete fBeamSigma;
  delete fDivSigma;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr)
{
  if (uiCmd == fBeamSigma) {
    fPrimaryGenerator->SetBeamSigma(fBeamSigma->GetNewDoubleValue(aStr));
  }
  else if (uiCmd == fDivSigma) {
    fPrimaryGenerator->SetDivergenceSigma(fDivSigma->GetNewDoubleValue(aStr));
  }
  else // should never happen
    G4Exception("PrimaryGeneratorAction_Messenger::SetNewValue", "FE_PrimGeneratorMessenger01", FatalException, "Command not found!\n");
}

endChR