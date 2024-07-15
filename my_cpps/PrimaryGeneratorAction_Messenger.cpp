#include "PrimaryGeneratorAction_Messenger.hpp"

//##########################################
//#######         VERSION 0.3        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

beginChR

//=========public ChR::PrimaryGeneratorAction_Messenger:: methods=========
PrimaryGeneratorAction_Messenger::PrimaryGeneratorAction_Messenger(PrimaryGeneratorAction* theGenerator)
:p_primaryGenerator(theGenerator){
	p_pGeneratorMessengerDir = new G4UIdirectory{ "/ChR_project/PrimaryGenerator/" };
	p_pGeneratorMessengerDir->SetGuidance("All commands related to primary generator action");

	p_beamSigma = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PrimaryGenerator/beamSigma", this };
	p_beamSigma->SetGuidance("Used to change the Gaussian sigma of the beam.");
	p_beamSigma->SetGuidance("If this value is set to '0', pencil-like beams are used. Otherwise, more realistic Gaussian beams are used.");
	p_beamSigma->SetGuidance("Gaussian distribution and this parameter are implemented as std::normal_distribution");
	p_beamSigma->SetParameterName("GaussSigma", true);
	p_beamSigma->SetDefaultUnit("um");
	p_beamSigma->SetDefaultValue(0.);
	p_beamSigma->SetRange("GaussSigma>0.");
	p_beamSigma->AvailableForStates(G4State_Idle); //keeping it hidden from the early master (there's no such an instance there)
	

	p_beamSigmaError = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PrimaryGenerator/beamSigmaError", this };
	p_beamSigmaError->SetGuidance("Used to change the error of Gaussian sigma of the beam.");
	p_beamSigmaError->SetGuidance("The sigma-parameter value used in std::normal_distribution is calculated as beamSigma +- normal distribution with beamSigmaError!");
	p_beamSigmaError->SetParameterName("beamSigmaError", true);
	p_beamSigmaError->SetDefaultUnit("um");
	p_beamSigmaError->SetDefaultValue(0.);
	p_beamSigmaError->SetRange("beamSigmaError>0.");
	p_beamSigmaError->AvailableForStates(G4State_Idle);

	p_divSigma = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PrimaryGenerator/thetaBeamDivergence", this };
	p_divSigma->SetGuidance("Used to change the PrimaryGeneratorAction::m_sinBeamDivergenceTheta.");
	p_divSigma->SetGuidance("It is used to simlate beam divergence as a normal distribution with the given sigma of the angle theta.");
	p_divSigma->SetParameterName("thetaBeamDivergence", true);
	p_divSigma->SetDefaultUnit("mrad");
	p_divSigma->SetDefaultValue(0.);
	p_divSigma->SetRange("thetaBeamDivergence>0.");
	p_divSigma->AvailableForStates(G4State_Idle);

	p_energy = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PrimaryGenerator/particleEnergy", this };
	p_energy->SetGuidance("Used to change energy of primary particles for ChR_project!");
	p_energy->SetGuidance("In this project, only heavy ions or electrons are used!");
	p_energy->SetGuidance("That means this parameter determines either total e- energy or energy per nucleon for heavy ions!");
	p_energy->SetGuidance("NOTE: I'm not setting range limits, but this project is meant for high energies!");
	p_energy->SetParameterName("primaryEnergy", true);
	p_energy->SetDefaultUnit("MeV");
	p_energy->SetDefaultValue(855.);
	p_energy->AvailableForStates(G4State_Idle);

	p_noOfParticles = new G4UIcmdWithAnInteger{ "/ChR_project/PrimaryGenerator/noOfPrimaryParticles", this };
	p_noOfParticles->SetGuidance("Used to change the number of primary particles per event!");
	p_noOfParticles->SetParameterName("noOfPrimaries", true);
	p_noOfParticles->SetDefaultValue(1);
	p_noOfParticles->SetRange("noOfPrimaries>0");
	p_noOfParticles->AvailableForStates(G4State_Idle);

	p_massNo = new G4UIcmdWithAnInteger{ "/ChR_project/PrimaryGenerator/massNo", this };
	p_massNo->SetGuidance("Used to set the mass number of primaries!");
	p_massNo->SetGuidance("NOTE #1: electrons will be used if you set massNo without setting atomicNo as well!");
	p_massNo->SetGuidance("NOTE #2: I did not care about physical meaning, i.e., I set range according to C++ bit-field type");
	p_massNo->SetParameterName("massNo", false);
	p_massNo->SetRange("massNo>=0 && massNo<512");
	p_massNo->AvailableForStates(G4State_Idle);

	p_atomicNo = new G4UIcmdWithAnInteger{ "/ChR_project/PrimaryGenerator/atomicNo", this };
	p_atomicNo->SetGuidance("Used to set the atomic number of primaries!");
	p_atomicNo->SetGuidance("NOTE #1: electrons will be used if you set atomicNo without setting massNo as well!");
	p_atomicNo->SetGuidance("NOTE #2: I did not care about physical meaning, i.e., I set range according to C++ bit-field type");
	p_atomicNo->SetParameterName("atomicNo", false);
	p_atomicNo->SetRange("atomicNo>=0 && atomicNo<128");
	p_atomicNo->AvailableForStates(G4State_Idle);
}

PrimaryGeneratorAction_Messenger::~PrimaryGeneratorAction_Messenger() {
	delete p_pGeneratorMessengerDir;
	delete p_beamSigma;
	delete p_beamSigmaError;
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
	else if (uiCmd == p_beamSigmaError) {
		p_primaryGenerator->SetSigErrParameter(p_beamSigmaError->GetNewDoubleValue(aStr));
	}
	else if (uiCmd == p_divSigma) {
		p_primaryGenerator->SetDivergenceSigma(p_divSigma->GetNewDoubleValue(aStr));
	}
	else if (uiCmd == p_energy) {
		p_primaryGenerator->SetParticleEnergy(p_energy->GetNewDoubleValue(aStr));
	}
	else if (uiCmd == p_noOfParticles) {
		p_primaryGenerator->SetNoOfParticles((const unsigned int)p_noOfParticles->ConvertToInt(aStr));
	}
	else if (uiCmd == p_massNo) {
		p_primaryGenerator->SetMassNo((const unsigned int)p_massNo->ConvertToInt(aStr));
	}
	else if (uiCmd == p_atomicNo) {
		p_primaryGenerator->SetAtomicNo((const unsigned int)p_atomicNo->ConvertToInt(aStr));
	}
	else
		G4Exception("PrimaryGeneratorAction_Messenger::SetNewValue", "WE1014", JustWarning, "Command not found!\n");
}

endChR