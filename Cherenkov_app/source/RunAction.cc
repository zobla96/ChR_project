//##########################################
//#######        VERSION 1.0.1       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

// user headers
#include "RunAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "SteppingAction.hh"
#include "StackingAction.hh"
// G4 headers (from ChR_process_lib)
#include "G4CherenkovProcess.hh"
#include "G4StandardCherenkovProcess.hh"
#include "G4ThinTargetChR_Model.hh"
// ...
#include "G4SystemOfUnits.hh"
#include "G4RunManager.hh"
#include "G4IonTable.hh"
#include "G4AnalysisManager.hh"
#include "G4ProcessTable.hh"
#include "G4ParticleGun.hh"
#include "G4Filesystem.hh"
#include "G4TaskGroup.hh"
#include "G4GlobalConfig.hh"
// std:: headers
#include <ctime>

#define PrintReadMeTrueOrFalse(someBoolValue)		\
	if (someBoolValue)								\
			outS << "true\n";						\
		else										\
			outS << "false\n"

beginChR

// object file scope
std::chrono::time_point<std::chrono::system_clock> o_beginOfRunAction;
std::chrono::time_point<std::chrono::system_clock> o_endOfRunAction;

//=========static RunAction.cc methods' declarations=========

static void BookAnalysisManager();

static void PrintRunDataIntoReadMe(std::ostream & outS);
static void Task_PrintPrimaryGenActionData(std::ostream & outS);
#ifdef standardRun
static void Task_SteppingActionData(std::ostream & outS);
 #ifdef boostEfficiency
static void Task_StackingActionData(std::ostream & outS);
 #endif // boostEfficiency
#endif // standardRun

//=========public ChR::RunAction:: methods=========

RunAction::RunAction() {
	G4RunManager::GetRunManager()->SetPrintProgress(1000000);
#if defined(standardRun) || defined(captureChRPhotonEnergyDistribution)
	BookAnalysisManager();
#endif // standardRun || captureChRPhotonEnergyDistribution
}

void RunAction::BeginOfRunAction(const G4Run*) {
#if defined(standardRun) || defined(captureChRPhotonEnergyDistribution)
	G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
 #ifdef standardRun
	// now set H1 titles - here because it should have the radiator angle in it
	std::ostringstream sStr{};
	sStr << std::fixed << std::setprecision(1) <<  g_detectorConstruction->GetRadiatorAngle() / deg;
	G4String aTitle{ sStr.str() };
	aTitle += " deg - Bin: ";
	for (G4int i = 0; i < analysisManager->GetNofH1s(); i++) {
		const G4double binSize =
			(analysisManager->GetH1Xmax(i) - analysisManager->GetH1Xmin(i)) / analysisManager->GetH1Nbins(i);
		sStr = std::ostringstream{};
		sStr << std::fixed << std::setprecision(2) << binSize;
		analysisManager->SetH1Title(i, aTitle + sStr.str() + " nm");
	}
 #endif // standardRun
	if (analysisManager->GetFileName().empty())
		analysisManager->SetFileName("the_results");
	analysisManager->OpenFile();
#endif // standardRun || captureChRPhotonEnergyDistribution
	if (!isMaster)
		PrepareRunData();
	else
		o_beginOfRunAction = std::chrono::system_clock::now();
}

void RunAction::EndOfRunAction(const G4Run*) {
#if defined(standardRun) || defined(captureChRPhotonEnergyDistribution)
	G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
	G4fs::path outFileName = analysisManager->GetFileName().c_str();
	analysisManager->Write();
	analysisManager->CloseFile();
	if (isMaster) {
		o_endOfRunAction = std::chrono::system_clock::now();
		G4fs::path currentPath = G4fs::current_path();
		G4fs::path newPath = currentPath;
		const long long currTime = std::chrono::time_point_cast<std::chrono::seconds>(o_endOfRunAction).time_since_epoch().count();
		newPath /= std::string{ outputFolderBegin } + std::to_string(currTime);
		G4fs::create_directory(newPath);

		// move .root
		currentPath /= analysisManager->GetFileName().c_str();
		if (G4fs::exists(currentPath)) {
			newPath /= analysisManager->GetFileName().c_str();
			G4fs::rename(currentPath, newPath); // potentially, catch(...) errors
			newPath.remove_filename();
		}
		currentPath.remove_filename();
		
		// print ReadMe about this run's used data
		newPath /= "ReadMe.txt";
		std::ofstream oFS{};
		oFS.open(newPath);
		newPath.remove_filename();
		if (oFS.is_open()) {
			PrintRunDataIntoReadMe(oFS);
			oFS.close();
		}
		else {
			G4cout << "\nFailed to open out ReadMe file:\n\n";
			PrintRunDataIntoReadMe(std::cout);
		}
	}
#endif // standardRun || captureChRPhotonEnergyDistribution
}

//=========private ChR::RunAction:: methods=========

void RunAction::PrepareRunData() {
	G4ParticleGun* theGun = g_primaryGenerator->GetParticleGun();
	G4ParticleDefinition* primaryDef = g_primaryGenerator->GetParticleGun()->GetParticleDefinition();
	if (primaryDef->GetParticleName() != "e-" && !G4IonTable::IsIon(primaryDef)) {
		const char* err = "The example supports the use of either electrons or high-energy ions as primaries!\n";
		G4Exception("RunAction::PrepareRunData", "FE_RunAction01", FatalException, err);
	}
	g_primaryGenerator->SetDistanceZ(g_detectorConstruction->GetRadiatorThickness() / std::cos(g_detectorConstruction->GetRadiatorAngle()) +
		5 * g_primaryGenerator->GetBeamSigma() * std::tan(g_detectorConstruction->GetRadiatorAngle()));
#ifdef boostEfficiency
	// Set delta phi and theta max and min
	if (g_primaryGenerator->GetBeamSigma() == 536. * um) {
		g_stackingAction->SetDeltaPhi(0.8);
		if (g_detectorConstruction->GetRadiatorAngle() == 22.0 * deg) {
			g_stackingAction->SetThetaMin(46.54 * deg);
			g_stackingAction->SetThetaMax(46.86 * deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 22.5 * deg) {
			g_stackingAction->SetThetaMin(46.65 * deg);
			g_stackingAction->SetThetaMax(46.97 * deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 23.0 * deg) {
			g_stackingAction->SetThetaMin(46.79 * deg);
			g_stackingAction->SetThetaMax(47.12 * deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 23.5 * deg) {
			g_stackingAction->SetThetaMin(46.93 * deg);
			g_stackingAction->SetThetaMax(47.24 * deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 24.0 * deg) {
			g_stackingAction->SetThetaMin(47.08 * deg);
			g_stackingAction->SetThetaMax(47.38 * deg);
		}
	}
	else if (g_primaryGenerator->GetBeamSigma() == 0. * um) {
		g_stackingAction->SetDeltaPhi(0.04);
		if (g_detectorConstruction->GetRadiatorAngle() == 22.0 * deg) {
			g_stackingAction->SetThetaMin(46.59 * deg);
			g_stackingAction->SetThetaMax(46.79 * deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 22.5 * deg) {
			g_stackingAction->SetThetaMin(46.74 * deg);
			g_stackingAction->SetThetaMax(46.93 * deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 23.0 * deg) {
			g_stackingAction->SetThetaMin(46.88 * deg);
			g_stackingAction->SetThetaMax(47.05 * deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 23.5 * deg) {
			g_stackingAction->SetThetaMin(47.03 * deg);
			g_stackingAction->SetThetaMax(47.17 * deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 24.0 * deg) {
			g_stackingAction->SetThetaMin(47.16 * deg);
			g_stackingAction->SetThetaMax(47.29 * deg);
		}
	}
	else
		g_stackingAction->SetDeltaPhi(1.0);
	// Make sure one is aware of what's done if non-default values are used
	if (!g_throwErrorForNonDefault)
		return;
	const PhysicsList* physList = dynamic_cast<const PhysicsList*>(G4RunManager::GetRunManager()->GetUserPhysicsList());
	if (g_detectorConstruction->GetRadiatorThickness() > 100. * um ||
		g_detectorConstruction->GetDetectorRadius() > 35. * um ||
		g_detectorConstruction->GetDetectorDistance() < 37.6 * cm ||
		g_primaryGenerator->GetParticleGun()->GetParticleDefinition()->GetParticleName() != "e-" ||
		g_primaryGenerator->GetBeamSigma() > 600. * um ||
		g_primaryGenerator->GetDivSigma() > 0. ||
		g_primaryGenerator->GetParticleGun()->GetParticleEnergy() < 855. * MeV ||
		physList->GetRadiatorRangeCuts_gamma() != 1000. * um ||
		physList->GetRadiatorRangeCuts_electron() != 1000. * um ||
		physList->GetRadiatorRangeCuts_positron() != 1000. * um ||
		physList->GetRadiatorRangeCuts_proton() != 1000. * um) {

		const char* err = "You want to use non-default parameters while haven't disabled\n"
			"the 'boostEfficiency' definition. That can be dangerous because\n"
			"of the StackingAction class which directs and modifies produced\n"
			"Cherenkov photons (to fly towards the detector). While that is\n"
			"beneficial, make sure that you change the emission angle accordingly\n"
			"and to read check what the principle is so you wouldn't obtain\n"
			"invalid results. On the other hand, the application execution\n"
			"has stopped just so you would have noticed this message. On the\n"
			"other hand, if you are aware of the risks and still want to proceed\n"
			"change the 'g_throwErrorForNonDefault' flag in 'DefsNConsts.hh'\n"
			"to 'false'";
		G4Exception("RunAction::LoadPrimaryGeneratorData", "FE_RunAction02", FatalException, err);
	}
#endif
}

//=========static RunAction.cc methods' definitions=========

static void BookAnalysisManager() {
	G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

	// create ntuples
	analysisManager->SetDefaultFileType("root");
	analysisManager->SetNtupleMerging(true);

	analysisManager->CreateNtuple("ChR_project", "The results");
	analysisManager->CreateNtupleIColumn("Event_ID");
	analysisManager->CreateNtupleDColumn("Energy");
	analysisManager->CreateNtupleDColumn("Wavelength");
#ifdef standardRun
	analysisManager->CreateNtupleDColumn("x-coordinate");
	analysisManager->CreateNtupleDColumn("y-coordinate");
#endif // standardRun
	analysisManager->FinishNtuple();

#ifdef standardRun
	// create H1s

	G4String histogramNames[3] = {
		"The ChR peak ID 0",
		"The ChR peak ID 1",
		"The ChR peak ID 2" };
	// range is from 300 to 800 nm because that's the range of registered RIndex values
	constexpr G4double minValue = 340.;
	constexpr G4double maxValue = 800.;

	for (size_t i = 0; i < 3; i++) {
 #ifdef boostEfficiency
		G4int binNo = (G4int)((maxValue - minValue) / (std::pow(2, i) * 0.25));
 #else
		G4int binNo = ((maxValue - minValue) / ((i + 1) * 1.));
 #endif // boostEfficiency
		G4int thisID = analysisManager->CreateH1(histogramNames[i], "", binNo, minValue, maxValue);
		analysisManager->SetH1XAxisTitle(thisID, "Wavelength [nm]");
		analysisManager->SetH1YAxisTitle(thisID, "Count");
	}
#endif // standardRun
}

static void PrintRunDataIntoReadMe(std::ostream& outS) {
	outS << G4RunManagerKernel::GetRunManagerKernel()->GetVersionString() << '\n'
		<< "The total number of events in this run was:  " << G4RunManager::GetRunManager()->GetNumberOfEventsToBeProcessed()
		<< "\nThe total number of threadPool threads was:  " << G4RunManager::GetRunManager()->GetNumberOfThreads() << '\n';
#ifdef standardRun
	outS << "\nThe total number of positive detections was: " << SteppingAction::GetNoOfDetections() << "\n\n";
#endif // standardRun
	std::time_t cTimeType = std::chrono::system_clock::to_time_t(o_beginOfRunAction);
	outS << std::left << std::setw(35) << "The master RunAction began at: " << std::ctime(&cTimeType);
	cTimeType = std::chrono::system_clock::to_time_t(o_endOfRunAction);
	outS << std::setw(35) << "The master RunAction finished at: " << std::ctime(&cTimeType);
	outS << std::right << std::setw(61) << std::setfill('=') << '\n' << "DetectorConstruction used values:\n";
	outS << std::setw(16) << std::setfill('+') << '\n' << std::setfill(' ') << std::left
		<< std::setw(13) << "Radiator" << "-> " << std::setw(16) << "Thickness:" << g_detectorConstruction->GetRadiatorThickness() / um << " um\n"
		<< std::setw(16) << ' ' << std::setprecision(1) << std::fixed << std::setw(16) << "Angle:" << g_detectorConstruction->GetRadiatorAngle() / deg << " deg\n"
		<< std::setw(16) << ' ' << std::setw(16) << "Used material: " << g_detectorConstruction->GetRadiatorMaterialName() << '\n'
		<< std::setw(16) << std::right << std::setfill('+') << '\n' << std::setfill(' ') << std::left
		<< std::setw(13) << "Detector" << "-> " << std::setw(16) << "Radius:" << g_detectorConstruction->GetDetectorRadius() / um << " um\n"
		<< std::setw(16) << ' ' << std::setw(16) << std::setprecision(1) << std::fixed << "Angle:" << g_detectorConstruction->GetDetectorAngle() / deg << " deg\n"
		<< std::setw(16) << ' ' << std::setw(16) << std::setprecision(4) << std::defaultfloat << "Distance:" << g_detectorConstruction->GetDetectorDistance() / mm << " mm\n"
		<< std::setw(16) << std::right << std::setfill('+') << '\n' << std::setfill(' ');
	outS << std::setw(61) << std::setfill('=') << '\n' << "PrimaryGeneratorAction used values:\n";
#ifdef G4MULTITHREADED
	G4TaskGroup<void> aTaskGroup{};
	aTaskGroup.exec([&outS] {Task_PrintPrimaryGenActionData(outS); }); //seems like the simplest way to wrap a task
	aTaskGroup.wait();
#else // !G4MULTITHREADED
	Task_PrintPrimaryGenActionData(outS);
#endif
	const PhysicsList* physList = dynamic_cast<const PhysicsList*>(G4RunManager::GetRunManager()->GetUserPhysicsList());
	outS << std::right << std::setw(61) << std::setfill('=') << '\n' << "PhysicsList used values:\n" << std::setfill(' ') << std::left
		<< std::setw(29) << "EM used model:" << physList->GetEMPhysicsInUse() << '\n'
		<< std::setw(29) << "Used optical physics:" << physList->GetOpticalPhysicsInUse() << '\n'
		<< std::setw(29) << "Radiator gamma cutValue:" << physList->GetRadiatorRangeCuts_gamma() / um << " um\n"
		<< std::setw(29) << "Radiator e- cutValue:" << physList->GetRadiatorRangeCuts_electron() / um << " um\n"
		<< std::setw(29) << "Radiator e+ cutValue:" << physList->GetRadiatorRangeCuts_positron() / um << " um\n"
		<< std::setw(29) << "Radiator proton cutValue:" << physList->GetRadiatorRangeCuts_proton() / um << " um\n";
	outS << std::right << std::setw(61) << std::setfill('=') << '\n' << "SteppingAction used values:\n" << std::setfill(' ') << std::left;
#ifdef standardRun
  #ifdef G4MULTITHREADED
	aTaskGroup.exec([&outS] { Task_SteppingActionData(outS); });
	aTaskGroup.wait();
  #else // !G4MULTITHREADED
	Task_SteppingActionData(outS);
  #endif // G4MULTITHREADED
#endif // standardRun
	// now Cherenkov models....
	if (physList->GetOpticalPhysicsInUse() == UseOptical::G4OpticalPhysics_option1) {
		outS << std::right << std::setw(61) << std::setfill('=') << '\n' << "G4StandardCherenkovProcess used values:\n" << std::setfill(' ') << std::left;
		const G4StandardCherenkovProcess* chProc = dynamic_cast<const G4StandardCherenkovProcess*>(G4ProcessTable::GetProcessTable()->FindProcess("StandardCherenkov", "e-"));
		outS << std::setw(21) << "Energy loss flag:";
		PrintReadMeTrueOrFalse(chProc->GetUseEnergyLoss());
		outS << std::setw(21) << "No of beta steps:" << std::to_string(chProc->GetNoOfBetaSteps()) << '\n';
	}
	else if (physList->GetOpticalPhysicsInUse() == UseOptical::G4OpticalPhysics_option2) {
		outS << std::right << std::setw(61) << std::setfill('=') << '\n' << "G4ExtraOpticalParameters used values:\n" << std::setfill(' ') << std::left;
		const auto extraOptParams = G4ExtraOpticalParameters::GetInstance();
		outS << "Cherenkov material data:\n";
		for (const auto& [key, value] : extraOptParams->GetChRMatData()) {
			outS << std::right << std::setw(21) << std::setfill('+') << '\n' << std::setfill(' ') << std::left
				<< std::setw(18) << key->GetName() << "-> "
				<< std::setw(21) << std::defaultfloat << "ChR model ID:" << value.m_executeModel << '\n'
				<< std::setw(21) << ' ' << std::setw(21) << "Exotic RIndex flag:";
			PrintReadMeTrueOrFalse(value.GetExoticRIndex());
			outS << std::setw(21) << ' ' << std::setw(21) << "Initial exotic flag:";
			PrintReadMeTrueOrFalse(value.GetExoticInitialFlag());
			outS << std::setw(21) << ' ' << std::setw(21) << "Min for axis:";
			if (value.GetMinAxis() == 0)
				outS << "x\n";
			else if (value.GetMinAxis() == 1)
				outS << "y\n";
			else if (value.GetMinAxis() == 2)
				outS << "z\n";
			else
				outS << "Not defined\n";
			outS << std::setw(21) << ' ' << std::setw(21) << "Half-thickness:";
			if (value.GetHalfThickness() < 0.)
				outS << "Not defined\n";
			else
				outS << value.GetHalfThickness() / um << " um\n";
			outS << std::setw(21) << ' ' << std::setw(21) << "Global center:";
			G4ThreeVector theMiddlePoint{ value.GetMiddlePointVec() };
			if (theMiddlePoint != G4ThreeVector{ DBL_MAX, DBL_MAX, DBL_MAX })
				outS << theMiddlePoint[0] << ", " << theMiddlePoint[1] << ", " << theMiddlePoint[2] << "\n";
			else
				outS << "Not defined\n";
		}
		const G4CherenkovProcess* chProc = dynamic_cast<const G4CherenkovProcess*>(G4ProcessTable::GetProcessTable()->FindProcess("Cherenkov", "e-"));
		outS << std::right << std::setw(61) << std::setfill('=') << '\n' << "Registered models in G4CherenkovProcess:\n" << std::setfill(' ') << std::left;
		for (size_t i = 0; i < chProc->GetNumberOfRegisteredModels(); i++) {
			const G4BaseChR_Model* aModel = chProc->GetChRModel(i);
			std::string modelID = "Model ID: " + std::to_string(i);
			outS << std::right << std::setw(21) << std::setfill('+') << '\n' << std::setfill(' ') << std::left
				<< std::setw(18) << modelID << "-> " << std::setw(24) << "Model name:" << aModel->GetChRModelName() << '\n'
				<< std::setw(21) << ' ' << std::setw(24) << "Finite thickness flag:";
			PrintReadMeTrueOrFalse(aModel->GetFiniteThicknessCondition());
			outS << std::setw(21) << ' ' << std::setw(24) << "Energy loss flag:";
			PrintReadMeTrueOrFalse(aModel->GetUseModelWithEnergyLoss());
			outS << std::setw(21) << ' ' << std::setw(24) << "No of beta steps:" << std::to_string(aModel->GetNoOfBetaSteps()) << '\n'
				<< std::setw(21) << ' ' << std::setw(24) << "Verbose level:" << std::to_string(aModel->GetVerboseLevel()) << '\n';
		}
	}
#ifdef boostEfficiency
	outS << std::right << std::setw(61) << std::setfill('=') << '\n' << "Limits in StackingAction:\n" << std::setfill(' ') << std::left;
  #ifdef G4MULTITHREADED
	aTaskGroup.exec([&outS] { Task_StackingActionData(outS); }); //seems like the simplest way to wrap a task
	aTaskGroup.wait();
  #else // !G4MULTITHREADED
	Task_StackingActionData(outS);
  #endif // G4MULTITHREADED
#endif // boostEfficiency
#ifdef followMinMaxValues
	outS << std::right << std::setw(61) << std::setfill('=') << '\n' << "The observed limits in this run were:\n" << std::setfill(' ') << std::left;
	outS << std::setprecision(6)
		<< std::setw(16) << "Phi min:" << g_minPhiValue / deg << " deg\n"
		<< std::setw(16) << "Phi max:" << g_maxPhiValue / deg << " deg\n"
		<< std::setw(16) << "Theta min:" << g_minThetaValue / deg << " deg\n"
		<< std::setw(16) << "Theta max:" << g_maxThetaValue / deg << " deg\n";
#endif // followMinMaxValues
}

static void Task_PrintPrimaryGenActionData(std::ostream& outS) {
	const G4ParticleGun* theGun = g_primaryGenerator->GetParticleGun();
	outS << std::left << std::setfill(' ') << std::setw(26) << "Used particle:";
	const G4String& particleName = theGun->GetParticleDefinition()->GetParticleName();
	outS << particleName;
	if (particleName == "e-")
		outS << '\n' << std::setw(26) << "Particle Energy:" << theGun->GetParticleEnergy() / MeV << " MeV\n";
	else /*it's an ion*/ {
		outS << " (Z = " << theGun->GetParticleDefinition()->GetAtomicNumber() << ", A = " << theGun->GetParticleDefinition()->GetAtomicMass() << ")\n"
			<< std::setw(26) << "Particle Energy:" << theGun->GetParticleEnergy() / (MeV * theGun->GetParticleDefinition()->GetAtomicMass()) << " MeV/nucleon\n";
	}
	outS << std::setw(26) << "Beam sigma:" << g_primaryGenerator->GetBeamSigma() / um << " um\n"
		<< std::setw(26) << "Beam divergence sigma:" << g_primaryGenerator->GetDivSigma() / mrad << " mrad\n"
		<< std::setw(26) << "Number of primaries:" << theGun->GetNumberOfParticles() << '\n';
}

#ifdef standardRun
static void Task_SteppingActionData(std::ostream& outS) {
	outS << std::setw(16) << "Verbose level:" << g_steppingAction->GetVerboseLevel() << '\n';
}
#endif // standardRun

#ifdef boostEfficiency
static void Task_StackingActionData(std::ostream& outS) {
	outS << std::setw(16) << "Delta phi:" << g_stackingAction->GetDeltaPhi() << " deg\n"
		<< std::setw(16) << "Theta min:" << g_stackingAction->GetThetaMin() / deg << " deg\n"
		<< std::setw(16) << "Theta max:" << g_stackingAction->GetThetaMax() / deg << " deg\n";
}
#endif // boostEfficiency

endChR