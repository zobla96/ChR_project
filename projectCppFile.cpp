//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "DefsNConsts.hpp"
#include "DetectorConstruction.hpp"
#include "PhysicsList.hpp"
#include "ActionInitialization.hpp"
//G4 headers
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4RunManagerFactory.hh"
#include "G4VisExecutive.hh"

int main(int argc, char** argv) {
	ChR::TimeBench<std::chrono::minutes> timeBNCH{ "main" };

	G4UIExecutive* uiExecutive = nullptr;
	if (argc == 1)
		uiExecutive = new G4UIExecutive{ argc, argv };

	CLHEP::HepRandom::getTheEngine()->setSeed(ChR::g_mtGen(), 0);

	//if needed one can set the step precision, but we don't care about it -
	//it's nothing other than std::ostream precision for floating_point type

	G4UIdirectory* uiChRProjectDir = new G4UIdirectory("/ChR_project/");
	uiChRProjectDir->SetGuidance("All commands related to this project - \"ChR_project\"");
	
	G4RunManager* runManager = G4RunManagerFactory::CreateRunManager();
	runManager->SetUserInitialization(ChR::DetectorConstruction::GetInstance());
	runManager->SetUserInitialization(new ChR::PhysicsList{});
	runManager->SetUserInitialization(new ChR::ActionInitialization{});

	G4UImanager* uiManager = G4UImanager::GetUIpointer();

	G4VisManager* visManager = nullptr;

	if (argc == 1) {
		visManager = new G4VisExecutive{};
		visManager->Initialize();
		//uiManager->ApplyCommand("/control/execute run2.mac");
		uiManager->ApplyCommand("/control/execute initialization.mac");
		uiExecutive->SessionStart();
	}
	else if (argc == 2) {
		G4String option1 = "/control/execute ";
		G4String option2 = argv[1];
		uiManager->ApplyCommand(option1 + option2);
	}
	else {
		G4cout << "The program doesn't include options argc > 2\n";
	}
	delete uiChRProjectDir;
	delete visManager;
	delete uiExecutive;
	delete runManager;

	return 0;
}