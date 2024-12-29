//##########################################
//#######        VERSION 1.0.0       #######
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

//=========public ChR::PrimaryGeneratorAction_Messenger:: methods=========

PrimaryGeneratorAction_Messenger::PrimaryGeneratorAction_Messenger(PrimaryGeneratorAction* theGenerator)
:p_primaryGenerator(theGenerator) {
	p_pGeneratorMessengerDir = new G4UIdirectory{ "/ChR_project/PrimaryGenerator/" };
	p_pGeneratorMessengerDir->SetGuidance("All commands related to primary generator action");

	p_beamSigma = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PrimaryGenerator/beamSigma", this };
	p_beamSigma->SetGuidance("Used to change the Gaussian sigma of the beam.");
	p_beamSigma->SetGuidance("If this value is set to '0.', pencil-like beams are used. Otherwise, more realistic Gaussian beams are used.");
	p_beamSigma->SetGuidance("Gaussian distribution and this parameter are implemented as std::normal_distribution");
	p_beamSigma->SetParameterName("GaussSigma", false);
	p_beamSigma->SetDefaultUnit("um");
	p_beamSigma->SetRange("GaussSigma>=0.");
	p_beamSigma->SetToBeBroadcasted(true);
	p_beamSigma->AvailableForStates(G4State_Idle); //keeping it hidden from the early master (there's no such an instance there)

	p_divSigma = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PrimaryGenerator/thetaBeamDivergence", this };
	p_divSigma->SetGuidance("Used to change the divergence angle as normal distribution.");
	p_divSigma->SetGuidance("It is used to simulate beam divergence as a normal distribution with the given sigma of the angle theta.");
	p_divSigma->SetParameterName("thetaBeamDivergence", false);
	p_divSigma->SetDefaultUnit("mrad");
	p_divSigma->SetRange("thetaBeamDivergence>=0.");
	p_divSigma->SetToBeBroadcasted(true);
	p_divSigma->AvailableForStates(G4State_Idle);
}

PrimaryGeneratorAction_Messenger::~PrimaryGeneratorAction_Messenger() {
	delete p_pGeneratorMessengerDir;
	delete p_beamSigma;
	delete p_divSigma;
}

void PrimaryGeneratorAction_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr) {
	if (uiCmd == p_beamSigma) {
		p_primaryGenerator->SetBeamSigma(p_beamSigma->GetNewDoubleValue(aStr));
	}
	else if (uiCmd == p_divSigma) {
		p_primaryGenerator->SetDivergenceSigma(p_divSigma->GetNewDoubleValue(aStr));
	}
	else // should never happen
		G4Exception("PrimaryGeneratorAction_Messenger::SetNewValue", "FE_PrimGeneratorMessenger01", FatalException, "Command not found!\n");
}

endChR