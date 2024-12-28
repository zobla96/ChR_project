//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "PhysicsList_Messenger.hpp"

beginChR

//=========public ChR::PhysicsList_Messenger:: methods=========

PhysicsList_Messenger::PhysicsList_Messenger(PhysicsList* phList)
:p_physicsList(phList) {
	p_physListDir = new G4UIdirectory{ "/ChR_project/PhysicsList/" };
	p_physListDir->SetGuidance("All commands related to ChR::PhysicsList");

	p_gammaRangeCut = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PhysicsList/gammaRangeCut", this };
	p_gammaRangeCut->SetGuidance("Used to change the gamma-cut value.");
	p_gammaRangeCut->SetParameterName("gammaCut", false);
	p_gammaRangeCut->SetDefaultUnit("um");
	p_gammaRangeCut->SetToBeBroadcasted(false);
	p_gammaRangeCut->AvailableForStates(G4State_PreInit);

	p_electronRangeCut = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PhysicsList/electronRangeCut", this };
	p_electronRangeCut->SetGuidance("Used to change the electron-cut value.");
	p_electronRangeCut->SetParameterName("electronCut", false);
	p_electronRangeCut->SetDefaultUnit("um");
	p_electronRangeCut->SetToBeBroadcasted(false);
	p_electronRangeCut->AvailableForStates(G4State_PreInit);

	p_positronRangeCut = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PhysicsList/positronRangeCut", this };
	p_positronRangeCut->SetGuidance("Used to change the positron-cut value.");
	p_positronRangeCut->SetParameterName("positronCut", false);
	p_positronRangeCut->SetDefaultUnit("um");
	p_positronRangeCut->SetToBeBroadcasted(false);
	p_positronRangeCut->AvailableForStates(G4State_PreInit);

	p_protonRangeCut = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/PhysicsList/protonRangeCut", this };
	p_protonRangeCut->SetGuidance("Used to change the proton-cut value.");
	p_protonRangeCut->SetParameterName("protonCut", false);
	p_protonRangeCut->SetDefaultUnit("um");
	p_protonRangeCut->SetToBeBroadcasted(false);
	p_protonRangeCut->AvailableForStates(G4State_PreInit);

	p_selectEmPhysics = new G4UIcmdWithAnInteger{ "/ChR_project/PhysicsList/selectEmPhysics", this };
	p_selectEmPhysics->SetGuidance("Used to change the EM physics that will be used.");
	p_selectEmPhysics->SetGuidance("There are various EM physics options, select a number of the desired EM physics from the list:");
	p_selectEmPhysics->SetGuidance("0 - G4EmStandardPhysics\n1 - G4EmStandardPhysics_option1\n2 - G4EmStandardPhysics_option2");
	p_selectEmPhysics->SetGuidance("3 - G4EmStandardPhysics_option3\n4 - G4EmStandardPhysics_option4\n5 - G4EmLivermorePhysics");
	p_selectEmPhysics->SetGuidance("6 - G4EmLowEPPhysics\n7 - G4EmPenelopePhysics\n8 - G4EmStandardPhysicsGS");
	p_selectEmPhysics->SetGuidance("9 - G4EmStandardPhysicsSS\n10 - G4EmStandardPhysicsWVI");
	p_selectEmPhysics->SetParameterName("EmPhysicsNoFromTheList", false);
	p_selectEmPhysics->SetRange("EmPhysicsNoFromTheList>=0 && EmPhysicsNoFromTheList<11");
	p_selectEmPhysics->SetToBeBroadcasted(false);
	p_selectEmPhysics->AvailableForStates(G4State_PreInit);
}

PhysicsList_Messenger::~PhysicsList_Messenger() {
	delete p_physListDir;
	delete p_gammaRangeCut;
	delete p_electronRangeCut;
	delete p_positronRangeCut;
	delete p_protonRangeCut;
	delete p_selectEmPhysics;
}

void PhysicsList_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr) {
	if (uiCmd == p_gammaRangeCut) {
		p_physicsList->SetRadiatorRangeCuts_gamma(p_gammaRangeCut->GetNewDoubleValue(aStr));
	}
	else if (uiCmd == p_electronRangeCut) {
		p_physicsList->SetRadiatorRangeCuts_electron(p_electronRangeCut->GetNewDoubleValue(aStr));
	}
	else if (uiCmd == p_positronRangeCut) {
		p_physicsList->SetRadiatorRangeCuts_positron(p_positronRangeCut->GetNewDoubleValue(aStr));
	}
	else if (uiCmd == p_protonRangeCut) {
		p_physicsList->SetRadiatorRangeCuts_proton(p_positronRangeCut->GetNewDoubleValue(aStr));
	}
	else if (uiCmd == p_selectEmPhysics) {
		int emIdNo = p_selectEmPhysics->ConvertToInt(aStr);
		switch (emIdNo) {
		case 0:
			p_physicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysics);
			break;
		case 1:
			p_physicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysics_option1);
			break;
		case 2:
			p_physicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysics_option2);
			break;
		case 3:
			p_physicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysics_option3);
			break;
		case 4:
			p_physicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysics_option4);
			break;
		case 5:
			p_physicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmLivermorePhysics);
			break;
		case 6:
			p_physicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmLowEPPhysics);
			break;
		case 7:
			p_physicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmPenelopePhysics);
			break;
		case 8:
			p_physicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysicsGS);
			break;
		case 9:
			p_physicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysicsSS);
			break;
		case 10:
			p_physicsList->ChangeEMPhysics(ChR::UseElectromagnetic::G4EmStandardPhysicsWVI);
			break;
		default: //shouldn't happen
			G4Exception("PhysicsList_Messenger::SetNewValue", "WE_PhysListMessenger01", JustWarning, "Somehow, selected physics not found!\n");
			break;
		}
	}
	else
		G4Exception("PhysicsList_Messenger::SetNewValue", "WE_PhysListMessenger02", JustWarning, "Command not found!\n");
}

endChR