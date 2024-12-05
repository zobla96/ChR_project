//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#include "PrimaryGeneratorAction_Messenger.hpp"

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

	p_energy = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PrimaryGenerator/particleEnergy", this };
	p_energy->SetGuidance("Used to change energy of primary particles for ChR_project!");
	p_energy->SetGuidance("In this project, only heavy ions or electrons are used!");
	p_energy->SetGuidance("That means this parameter determines either total e- energy or energy per nucleon for heavy ions!");
	p_energy->SetGuidance("NOTE: I'm not setting range limits, but this project is meant for high energies!");
	p_energy->SetParameterName("primaryEnergy", false);
	p_energy->SetDefaultUnit("MeV");
	p_energy->SetToBeBroadcasted(true);
	p_energy->AvailableForStates(G4State_Idle);

	p_noOfParticles = new G4UIcmdWithAnInteger{ "/ChR_project/PrimaryGenerator/noOfPrimaryParticles", this };
	p_noOfParticles->SetGuidance("Used to change the number of primary particles per event!");
	p_noOfParticles->SetParameterName("noOfPrimaries", false);
	p_noOfParticles->SetRange("noOfPrimaries>0");
	p_noOfParticles->SetToBeBroadcasted(true);
	p_noOfParticles->AvailableForStates(G4State_Idle);

	p_massNo = new G4UIcmdWithAnInteger{ "/ChR_project/PrimaryGenerator/massNo", this };
	p_massNo->SetGuidance("Used to set the mass number of primaries!");
	p_massNo->SetGuidance("NOTE #1: electrons will be used if you set massNo without setting atomicNo as well!");
	p_massNo->SetGuidance("NOTE #2: I did not care about physical meaning, i.e., I set range according to C++ bit-field type");
	p_massNo->SetParameterName("massNo", false);
	p_massNo->SetRange("massNo>=0 && massNo<512");
	p_massNo->SetToBeBroadcasted(true);
	p_massNo->AvailableForStates(G4State_Idle);

	p_atomicNo = new G4UIcmdWithAnInteger{ "/ChR_project/PrimaryGenerator/atomicNo", this };
	p_atomicNo->SetGuidance("Used to set the atomic number of primaries!");
	p_atomicNo->SetGuidance("NOTE #1: electrons will be used if you set atomicNo without setting massNo as well!");
	p_atomicNo->SetGuidance("NOTE #2: I did not care about physical meaning, i.e., I set range according to C++ bit-field type");
	p_atomicNo->SetParameterName("atomicNo", false);
	p_atomicNo->SetRange("atomicNo>=0 && atomicNo<128");
	p_atomicNo->SetToBeBroadcasted(true);
	p_atomicNo->AvailableForStates(G4State_Idle);
}

PrimaryGeneratorAction_Messenger::~PrimaryGeneratorAction_Messenger() {
	delete p_pGeneratorMessengerDir;
	delete p_beamSigma;
	delete p_divSigma;
	delete p_energy;
	delete p_noOfParticles;
	delete p_massNo;
	delete p_atomicNo;
}

void PrimaryGeneratorAction_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr) {
	if (uiCmd == p_beamSigma) {
		p_primaryGenerator->SetBeamSigma(p_beamSigma->GetNewDoubleValue(aStr));
	}
	else if (uiCmd == p_divSigma) {
		p_primaryGenerator->SetDivergenceSigma(p_divSigma->GetNewDoubleValue(aStr));
	}
	else if (uiCmd == p_energy) {
		p_primaryGenerator->SetParticleEnergy(p_energy->GetNewDoubleValue(aStr));
	}
	else if (uiCmd == p_noOfParticles) {
		p_primaryGenerator->SetNoOfParticles(p_noOfParticles->ConvertToInt(aStr));
	}
	else if (uiCmd == p_massNo) {
		p_primaryGenerator->SetMassNo((const unsigned short)p_massNo->ConvertToInt(aStr));
	}
	else if (uiCmd == p_atomicNo) {
		p_primaryGenerator->SetAtomicNo((const unsigned short)p_atomicNo->ConvertToInt(aStr));
	}
	else //just in case of some bug, but it can be removed
		G4Exception("PrimaryGeneratorAction_Messenger::SetNewValue", "WE_PrimGeneratorMessenger01", JustWarning, "Command not found!\n");
}

endChR