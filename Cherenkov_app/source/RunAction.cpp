//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#include "RunAction.hpp"
#include "ProcessCsvData.hpp"
#include "G4CherenkovProcess.hh"
#include "G4StandardCherenkovProcess.hh"
#include "G4ThinTargetChR_Model.hh"

#define PrintReadMeTrueOrFalse(someBoolValue)		\
	if (someBoolValue)								\
			outFS << "true\n";						\
		else										\
			outFS << "false\n"

beginChR

inline std::mutex o_RunManagerMutex;
std::vector<EventAction::LayerData> o_LayerDataVec; //object(file) scope
std::chrono::time_point<std::chrono::system_clock> o_beginOfRunAction;
std::chrono::time_point<std::chrono::system_clock> o_endOfRunAction;

static void Task_PrintPrimaryGenActionData(std::ofstream& outFS) {
	outFS << std::left << std::setfill(' ') << std::setw(26) << "Used particle:";
	if (g_primaryGenerator->GetAtomicNo() != 0 && g_primaryGenerator->GetMassNo() != 0) {
		outFS << "GenericIon (Z = " << g_primaryGenerator->GetAtomicNo() << ", A = " << g_primaryGenerator->GetMassNo() << ")\n"
			<< std::setw(26) << "Full particle name:" << g_primaryGenerator->GetParticleGun()->GetParticleDefinition()->GetParticleName() << '\n'
			<< std::setw(26) << "Particle Energy:" << g_primaryGenerator->GetParticleEnergy() / MeV << " MeV/nucleon\n";
	}
	else {
		outFS << "e-\n"
			<< std::setw(26) << "Particle Energy:" << g_primaryGenerator->GetParticleEnergy() / MeV << " MeV\n";
	}
	outFS << std::setw(26) << "Beam sigma:" << g_primaryGenerator->GetBeamSigma() / um << " um\n"
		<< std::setw(26) << "Beam divergence sigma:" << g_primaryGenerator->GetDivSigma() / mrad << " mrad\n"
		<< std::setw(26) << "Number of primaries:" << g_primaryGenerator->GetNoOfParticles() << '\n';
}

#ifdef standardRun
static void Task_SteppingActionData(std::ofstream& outFS) {
	outFS << std::setw(16) << "Verbose level:" << std::to_string(g_steppingAction->GetVerboseLevel()) << '\n';
}
#endif // standardRun

#ifdef boostEfficiency
static void Task_StackingActionData(std::ofstream& outFS) {
	outFS << std::setw(16) << "Delta phi:" << g_stackingAction->GetDeltaPhi() << " deg\n"
		<< std::setw(16) << "Theta min:" << g_stackingAction->GetThetaMin() / deg << " deg\n"
		<< std::setw(16) << "Theta max:" << g_stackingAction->GetThetaMax() / deg << " deg\n";
}
#endif // boostEfficiency

template <>
void ProcessCsvData<int, double, double, double, double>::ReadMePrintAboutCurrentProjectData(std::ofstream& outFS) {
	outFS << "\nThe total number of positive detections was: " << m_dataVec.size() << "\n\n";
	std::time_t cTimeType = std::chrono::system_clock::to_time_t(o_beginOfRunAction);
	outFS << std::left << std::setw(35) << "The master RunAction began at: " << std::ctime(&cTimeType);
	cTimeType = std::chrono::system_clock::to_time_t(o_endOfRunAction);
	outFS << std::setw(35) << "The master RunAction finished at: " << std::ctime(&cTimeType);
	outFS << std::right << std::setw(61) << std::setfill('=') << '\n' << "DetectorConstruction used values:\n";
	outFS << std::setw(16) << std::setfill('+') << '\n' << std::setfill(' ') << std::left
		<< std::setw(13) << "Radiator" << "-> " << std::setw(16) << "Thickness:" << g_detectorConstruction->GetRadiatorThickness() / um << " um\n"
		<< std::setw(16) << ' ' << std::setprecision(1) << std::fixed << std::setw(16) << "Angle:" << g_detectorConstruction->GetRadiatorAngle() / deg << " deg\n"
		<< std::setw(16) << ' ' << std::setw(16) << std::setprecision(4) << std::defaultfloat << "No of layers:" << std::to_string(g_detectorConstruction->GetNoOfRadLayers()) << '\n'
		<< std::setw(16) << ' ' << std::setw(16) << "Used material: " << g_detectorConstruction->GetRadiatorMaterialName() << '\n'
		<< std::setw(16) << std::right << std::setfill('+') << '\n' << std::setfill(' ') << std::left
		<< std::setw(13) << "Detector" << "-> " << std::setw(16) << "Radius:" << g_detectorConstruction->GetDetectorRadius() / um << " um\n"
		<< std::setw(16) << ' ' << std::setw(16) << std::setprecision(1) << std::fixed << "Angle:" << g_detectorConstruction->GetDetectorAngle() / deg << " deg\n"
		<< std::setw(16) << ' ' << std::setw(16) << std::setprecision(4) << std::defaultfloat << "Distance:" << g_detectorConstruction->GetDetectorDistance() / mm << " mm\n"
		<< std::setw(16) << std::right << std::setfill('+') << '\n' << std::setfill(' ') << std::left
		<< std::setw(16) << "Verbose level:" << std::to_string(g_detectorConstruction->GetVerboseLevel()) << '\n'
		<< std::setw(16) << "Overlap check:";
	PrintReadMeTrueOrFalse(g_detectorConstruction->GetCheckOverlap());
	outFS << std::right << std::setw(61) << std::setfill('=') << '\n' << "PrimaryGeneratorAction used values:\n";
	m_taskGroup.exec([&outFS] {Task_PrintPrimaryGenActionData(outFS); }); //seems like the simplest way to wrap a task
	m_taskGroup.wait();
	const PhysicsList* physList = dynamic_cast<const PhysicsList*>(G4RunManager::GetRunManager()->GetUserPhysicsList());
	outFS << std::right << std::setw(61) << std::setfill('=') << '\n' << "PhysicsList used values:\n" << std::setfill(' ') << std::left
		<< std::setw(29) << "EM used model:" << physList->GetEMPhysicsInUse() << '\n'
		<< std::setw(29) << "Used optical physics:";
	std::string opticalPhysicsName;
	if (physList->GetPhysics("Optical"))
		opticalPhysicsName = "Optical";
	else if (physList->GetPhysics("OpticalPhysics_op1"))
		opticalPhysicsName = "OpticalPhysics_op1";
	else if (physList->GetPhysics("OpticalPhysics_op2"))
		opticalPhysicsName = "OpticalPhysics_op2";
	else
		opticalPhysicsName = "No registered optical physics!";
	outFS << opticalPhysicsName << '\n';
	outFS << std::setw(29) << "Radiator gamma cutValue:" << physList->GetRadiatorRangeCuts_gamma() / um << " um\n"
		<< std::setw(29) << "Radiator e- cutValue:" << physList->GetRadiatorRangeCuts_electron() / um << " um\n"
		<< std::setw(29) << "Radiator e+ cutValue:" << physList->GetRadiatorRangeCuts_positron() / um << " um\n"
		<< std::setw(29) << "Radiator proton cutValue:" << physList->GetRadiatorRangeCuts_proton() / um << " um\n";
	outFS << std::right << std::setw(61) << std::setfill('=') << '\n' << "SteppingAction used values:\n" << std::setfill(' ') << std::left;
#ifdef standardRun
	m_taskGroup.exec([&outFS] { Task_SteppingActionData(outFS); });
	m_taskGroup.wait();
#endif // standardRun
	//now Cherenkov models....
	if (opticalPhysicsName == "OpticalPhysics_op1") {
		outFS << std::right << std::setw(61) << std::setfill('=') << '\n' << "G4StandardCherenkovProcess used values:\n" << std::setfill(' ') << std::left;
		const G4StandardCherenkovProcess* chProc = dynamic_cast<const G4StandardCherenkovProcess*>(G4ProcessTable::GetProcessTable()->FindProcess("StandardCherenkov", "e-"));
		outFS << std::setw(21) << "Energy loss flag:";
		PrintReadMeTrueOrFalse(chProc->GetUseEnergyLoss());
		outFS << std::setw(21) << "No of beta steps:" << std::to_string(chProc->GetNoOfBetaSteps()) << '\n';
	}
	else if (opticalPhysicsName == "OpticalPhysics_op2") {
		outFS << std::right << std::setw(61) << std::setfill('=') << '\n' << "G4ExtraOpticalParameters used values:\n" << std::setfill(' ') << std::left;
		const auto extraOptParams = G4ExtraOpticalParameters::GetInstance();
		outFS << "Cherenkov material data:\n";
		for (const auto& [key, value] : extraOptParams->GetChRMatData()) {
			outFS << std::right << std::setw(21) << std::setfill('+') << '\n' << std::setfill(' ') << std::left
				<< std::setw(18) << key->GetName() << "-> "
				<< std::setw(21) << std::defaultfloat << "ChR model ID:" << value.m_executeModel << '\n'
				<< std::setw(21) << ' ' << std::setw(21) << "Exotic RIndex flag:";
			PrintReadMeTrueOrFalse(value.GetExoticRIndex());
			outFS << std::setw(21) << ' ' << std::setw(21) << "Initial exotic flag:";
			PrintReadMeTrueOrFalse(value.GetExoticInitialFlag());
			outFS << std::setw(21) << ' ' << std::setw(21) << "Min for axis:";
			if (value.GetMinAxis() == 0)
				outFS << "x\n";
			else if (value.GetMinAxis() == 1)
				outFS << "y\n";
			else if (value.GetMinAxis() == 2)
				outFS << "z\n";
			else
				outFS << "Not defined\n";
			outFS << std::setw(21) << ' ' << std::setw(21) << "Half-thickness:";
			if (value.GetHalfThickness() < 0.)
				outFS << "Not defined\n";
			else
				outFS << value.GetHalfThickness() / um << " um\n";
			outFS << std::setw(21) << ' ' << std::setw(21) << "Global center:";
			G4ThreeVector theMiddlePoint{ value.GetMiddlePointVec() };
			if (theMiddlePoint != G4ThreeVector{ DBL_MAX, DBL_MAX, DBL_MAX })
				outFS << theMiddlePoint[0] << ", " << theMiddlePoint[1] << ", " << theMiddlePoint[2] << "\n";
			else
				outFS << "Not defined\n";
		}
		const G4CherenkovProcess* chProc = dynamic_cast<const G4CherenkovProcess*>(G4ProcessTable::GetProcessTable()->FindProcess("Cherenkov", "e-"));
		outFS << std::right << std::setw(61) << std::setfill('=') << '\n' << "Registered models in G4CherenkovProcess:\n" << std::setfill(' ') << std::left;
		for (size_t i = 0; i < chProc->GetNumberOfRegisteredModels(); i++) {
			const G4BaseChR_Model* aModel = chProc->GetChRModel(i);
			std::string modelID = "Model ID: " + std::to_string(i);
			outFS << std::right << std::setw(21) << std::setfill('+') << '\n' << std::setfill(' ') << std::left
				<< std::setw(18) << modelID << "-> " << std::setw(24) << "Model name:" << aModel->GetChRModelName() << '\n'
				<< std::setw(21) << ' ' << std::setw(24) << "Finite thickness flag:";
			PrintReadMeTrueOrFalse(aModel->GetFiniteThicknessCondition());
			outFS << std::setw(21) << ' ' << std::setw(24) << "Energy loss flag:";
			PrintReadMeTrueOrFalse(aModel->GetUseModelWithEnergyLoss());
			outFS << std::setw(21) << ' ' << std::setw(24) << "No of beta steps:" << std::to_string(aModel->GetNoOfBetaSteps()) << '\n'
				<< std::setw(21) << ' ' << std::setw(24) << "Verbose level:" << std::to_string(aModel->GetVerboseLevel()) << '\n';
		}
	}
#ifdef boostEfficiency
	outFS << std::right << std::setw(61) << std::setfill('=') << '\n' << "Limits in StackingAction:\n" << std::setfill(' ') << std::left;
	m_taskGroup.exec([&outFS] { Task_StackingActionData(outFS); }); //seems like the simplest way to wrap a task
	m_taskGroup.wait();
#endif // boostEfficiency
#ifdef followMinMaxValues
	outFS << std::right << std::setw(61) << std::setfill('=') << '\n' << "The observed limits in this run were:\n" << std::setfill(' ') << std::left;
	outFS << std::setprecision(6)
		<< std::setw(16) << "Phi min:" << g_minPhiValue / deg << " deg\n"
		<< std::setw(16) << "Phi max:" << g_maxPhiValue / deg << " deg\n"
		<< std::setw(16) << "Theta min:" << g_minThetaValue / deg << " deg\n"
		<< std::setw(16) << "Theta max:" << g_maxThetaValue / deg << " deg\n";
#endif // followMinMaxValues
}

//=========public ChR::RunAction:: methods=========

RunAction::RunAction() {
	G4RunManager::GetRunManager()->SetPrintProgress(1000000);

	G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
	// remove the following comments if using root files
	//analysisManager->SetDefaultFileType("root");
	//analysisManager->SetNtupleMerging(true);

	analysisManager->CreateNtuple("ChR_project", "The results");
	analysisManager->CreateNtupleIColumn("Event_ID");
	analysisManager->CreateNtupleDColumn("Energy");
	analysisManager->CreateNtupleDColumn("Wavelength");
	analysisManager->CreateNtupleDColumn("x-coordinate");
	analysisManager->CreateNtupleDColumn("y-coordinate");
	analysisManager->FinishNtuple();
}

void RunAction::BeginOfRunAction(const G4Run*) {
	G4AnalysisManager::Instance()->OpenFile("The_results.csv");
	if (!isMaster)
		LoadPrimaryGeneratorData();
	else
		o_beginOfRunAction = std::chrono::system_clock::now();
}

void RunAction::EndOfRunAction(const G4Run*) {
	G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
	//analysisManager.GetFileType ain't extension, but an empty string... don't see a method to obtain the info o.O
	G4fs::path outFileName = analysisManager->GetFileName().c_str();
	analysisManager->Write();
	analysisManager->CloseFile();
#ifdef standardRun
	if (!isMaster) {
		if (g_detectorConstruction->GetNoOfRadLayers() > 1) {
			const std::vector<EventAction::LayerData>& layerData = g_eventAction->GetLayerDataVec();
			std::lock_guard lck(o_RunManagerMutex);
			o_LayerDataVec.reserve(layerData.size());
			if (o_LayerDataVec.size() == 0) {
				for (size_t i = 0; i < layerData.size(); i++)
					o_LayerDataVec.emplace_back();
			}
			try {
				for (size_t i = 0; i < layerData.size(); i++)
					o_LayerDataVec.at(i) += layerData.at(i);
			}
			catch (std::out_of_range) {
				G4Exception("RunAction::EndOfRunAction", "FE_RunAction01", FatalException, "Various worker threads with various sizes of layerDataVectors!\n");
			}
		}
	}
	else {
		o_endOfRunAction = std::chrono::system_clock::now();
		if (g_detectorConstruction->GetNoOfRadLayers() > 1) {
			//save o_LayerDataVec
			std::ofstream runOFS;
			runOFS.open("LostEPerLayer.csv", std::ios::out | std::ios::trunc);
			if (!runOFS)
				G4Exception("RunAction::EndOfRunAction", "WE_RunAction01", JustWarning, "Didn't manage to open \"LostEPerLayer.csv\"");
			else {
				for (auto& forPrinting : o_LayerDataVec)
					runOFS << forPrinting.m_count << ',' << forPrinting.m_lostE / MeV << ',' << forPrinting.m_stepLng / um << '\n';
				runOFS.close();
			}
			//end of saving o_LayerDataVec
			//============================================
		}
		//do initial processing of raw data if csv is in use
		if (outFileName.extension() == ".csv") {
			TimeBench<std::chrono::microseconds> timeBNCH{ "RunAction::EndOfRunAction - processCSV" };
			ProcessCsvData<int, double, double, double, double> processCsv{ "The_results", "ChR_project" };
			processCsv.MoveAFileToFinalDestination("LostEPerLayer.csv");
			processCsv.Process_N_D_Data<2>(std::vector<double>{0.25, 0.5, 1., 2., 3.}, "Eff_PeakWaveLng", "efficiency.csv");
			processCsv.Process_N_D_Data<2>(std::vector<double>{0.25, 0.5, 1., 2., 3.}, "PeakWaveLng");
			//processCsv.Process_N_D_Data<3, 4>(std::vector<double>{3.}, "DetDist");
		}
	}
#else
	if (isMaster) {
		TimeBench<std::chrono::microseconds> timeBNCH{ "RunAction::EndOfRunAction - processCSV" };
		ProcessCsvData<int, double, double, double, double> processCsv{ "The_results", "ChR_project" };
		processCsv.Process_N_D_Data<1>(std::vector<double>{0.01}, "PeakWaveLng");
	}
#endif // standardRun
}

//=========private ChR::RunAction:: methods=========

void RunAction::LoadPrimaryGeneratorData() {
	G4ParticleGun* theGun = g_primaryGenerator->GetParticleGun();
	if (g_primaryGenerator->GetMassNo() != 0 && g_primaryGenerator->GetAtomicNo() != 0) {
		theGun->SetParticleDefinition(G4IonTable::GetIonTable()->GetIon((int)g_primaryGenerator->GetAtomicNo(), (int)g_primaryGenerator->GetMassNo(), 0.));
		theGun->SetParticleCharge(g_primaryGenerator->GetAtomicNo() * eplus);
		theGun->SetParticleEnergy(g_primaryGenerator->GetMassNo() * g_primaryGenerator->GetParticleEnergy());
	}
	else {
		theGun->SetParticleDefinition(G4ParticleTable::GetParticleTable()->FindParticle("e-"));
		theGun->SetParticleEnergy(g_primaryGenerator->GetParticleEnergy());
	}
	theGun->SetNumberOfParticles(g_primaryGenerator->GetNoOfParticles());
	g_primaryGenerator->SetDistanceZ(g_detectorConstruction->GetRadiatorThickness() / std::cos(g_detectorConstruction->GetRadiatorAngle()) +
		5 * g_primaryGenerator->GetBeamSigma() * std::tan(g_detectorConstruction->GetRadiatorAngle()));
#ifdef boostEfficiency
	// Set delta phi and theta max and min
	if (g_primaryGenerator->GetBeamSigma() == 536._um) {
		g_stackingAction->SetDeltaPhi(0.8);
		if (g_detectorConstruction->GetRadiatorAngle() == 22.0_deg) {
			g_stackingAction->SetThetaMin(46.54_deg);
			g_stackingAction->SetThetaMax(46.86_deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 22.5_deg) {
			g_stackingAction->SetThetaMin(46.65_deg);
			g_stackingAction->SetThetaMax(46.97_deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 23.0_deg) {
			g_stackingAction->SetThetaMin(46.79_deg);
			g_stackingAction->SetThetaMax(47.12_deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 23.5_deg) {
			g_stackingAction->SetThetaMin(46.93_deg);
			g_stackingAction->SetThetaMax(47.24_deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 24.0_deg) {
			g_stackingAction->SetThetaMin(47.08_deg);
			g_stackingAction->SetThetaMax(47.38_deg);
		}
	}
	else if (g_primaryGenerator->GetBeamSigma() == 0._um) {
		g_stackingAction->SetDeltaPhi(0.04);
		if (g_detectorConstruction->GetRadiatorAngle() == 22.0_deg) {
			g_stackingAction->SetThetaMin(46.59_deg);
			g_stackingAction->SetThetaMax(46.79_deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 22.5_deg) {
			g_stackingAction->SetThetaMin(46.74_deg);
			g_stackingAction->SetThetaMax(46.93_deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 23.0_deg) {
			g_stackingAction->SetThetaMin(46.88_deg);
			g_stackingAction->SetThetaMax(47.05_deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 23.5_deg) {
			g_stackingAction->SetThetaMin(47.03_deg);
			g_stackingAction->SetThetaMax(47.17_deg);
		}
		else if (g_detectorConstruction->GetRadiatorAngle() == 24.0_deg) {
			g_stackingAction->SetThetaMin(47.16_deg);
			g_stackingAction->SetThetaMax(47.29_deg);
		}
	}
	else
		g_stackingAction->SetDeltaPhi(1.0);
	// Make sure one is aware of what's done if non-default values are used
	if (!g_throwErrorForNonDefault)
		return;
	const PhysicsList* physList = dynamic_cast<const PhysicsList*>(G4RunManager::GetRunManager()->GetUserPhysicsList());
	if (g_detectorConstruction->GetRadiatorThickness() > 100._um ||
		g_detectorConstruction->GetDetectorRadius() > 35._um ||
		g_detectorConstruction->GetDetectorDistance() < 37.6_cm ||
		g_primaryGenerator->GetParticleGun()->GetParticleDefinition()->GetParticleName() != "e-" ||
		g_primaryGenerator->GetBeamSigma() > 600._um ||
		g_primaryGenerator->GetDivSigma() > 0. ||
		g_primaryGenerator->GetParticleEnergy() < 855._MeV ||
		physList->GetRadiatorRangeCuts_gamma() != 1000._um ||
		physList->GetRadiatorRangeCuts_electron() != 1000._um ||
		physList->GetRadiatorRangeCuts_positron() != 1000._um ||
		physList->GetRadiatorRangeCuts_proton() != 1000._um) {

		const char* err = "You want to use non-default parameters while haven't disabled\n"
			"the 'boostEfficiency' definition. That can be dangerous because\n"
			"of the StackingAction class which directs and modifies produced\n"
			"Cherenkov photons (to fly towards the detector). While that is\n"
			"beneficial, make sure that you change the emission angle accordingly\n"
			"and to read check what the principle is so you wouldn't obtain\n"
			"invalid results. On the other hand, the application execution\n"
			"has stopped just so you would have noticed this message. On the\n"
			"other hand, if you are aware of the risks and still want to proceed\n"
			"change the 'g_throwErrorForNonDefault' flag in 'DefsNConsts.hpp'\n"
			"to 'false'";
		G4Exception("RunAction::LoadPrimaryGeneratorData", "FE_RunAction02", FatalException, err);
	}
#endif
}

endChR