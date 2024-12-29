//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "DefsNConsts.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "ActionInitialization.hh"
//G4 headers
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4RunManagerFactory.hh"
#include "G4VisExecutive.hh"

int main(int argc, char** argv) {
	{
#ifdef standardRun
  #ifdef boostEfficiency
		ChR::TimeBench<std::chrono::seconds> timeBNCH{ "main" };
  #else
		ChR::TimeBench<std::chrono::minutes> timeBNCH{ "main" };
  #endif // boostEfficiency

#else // !standardRun
		ChR::TimeBench<std::chrono::milliseconds> timeBNCH{ "main" };
#endif // standardRun

		G4UIExecutive* uiExecutive = nullptr;
		if (argc == 1)
			uiExecutive = new G4UIExecutive{ argc, argv };

		CLHEP::HepRandom::getTheEngine()->setSeed((long)ChR::g_mtGen(), 0);

		G4UIdirectory* uiChRProjectDir = new G4UIdirectory("/ChR_project/");
		uiChRProjectDir->SetGuidance("All commands related to this project (application) - \"ChR_project\"");

		G4RunManager* runManager = G4RunManagerFactory::CreateRunManager();
		ChR::g_detectorConstruction = new ChR::DetectorConstruction{};
		runManager->SetUserInitialization(ChR::g_detectorConstruction);
		runManager->SetUserInitialization(new ChR::PhysicsList{ 0 });
		runManager->SetUserInitialization(new ChR::ActionInitialization{});

		G4UImanager* uiManager = G4UImanager::GetUIpointer();

		G4VisManager* visManager = nullptr;

		if (argc == 1) {
#ifndef standardRun
			//leave only transportation and Cherenkov process for e-
			uiManager->ApplyCommand("/process/inactivate Scintillation");
			uiManager->ApplyCommand("/process/inactivate electronNuclear");
			uiManager->ApplyCommand("/process/inactivate CoulombScat");
			uiManager->ApplyCommand("/process/inactivate eBrem");
			uiManager->ApplyCommand("/process/inactivate eIoni");
			uiManager->ApplyCommand("/process/inactivate msc");
#endif // !standardRun
			visManager = new G4VisExecutive{};
			visManager->Initialize();
			//uiManager->ApplyCommand("/control/execute run2.mac");
			uiManager->ApplyCommand("/control/execute initialization.mac");
			uiExecutive->SessionStart();
		}
		else if (argc == 2) {
#ifndef standardRun
			//leave only transportation and Cherenkov process for e-
			uiManager->ApplyCommand("/process/inactivate Scintillation");
			uiManager->ApplyCommand("/process/inactivate electronNuclear");
			uiManager->ApplyCommand("/process/inactivate CoulombScat");
			uiManager->ApplyCommand("/process/inactivate eBrem");
			uiManager->ApplyCommand("/process/inactivate eIoni");
			uiManager->ApplyCommand("/process/inactivate msc");
#endif // !standardRun
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
	}
	//std::cin.get();

	return 0;
}