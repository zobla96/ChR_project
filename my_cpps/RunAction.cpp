#include "RunAction.hpp"

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

beginChR

inline std::mutex o_RunManagerMutex;
std::vector<EventAction::LayerData> o_LayerDataVec; //object(file) scope
std::chrono::time_point<std::chrono::system_clock> o_beginOfRunAction;
std::chrono::time_point<std::chrono::system_clock> o_endOfRunAction;

void Task_PrintPrimaryGenActionData(std::ofstream& outFS) {
	PrimaryGeneratorAction* genAction = PrimaryGeneratorAction::GetInstance();
	outFS << std::left << std::setfill(' ') << std::setw(26) << "Used particle:";
	if (genAction->GetAtomicNo() != 0 && genAction->GetMassNo() != 0) {
		outFS << "GenericIon (Z = " << genAction->GetAtomicNo() << ", A = " << genAction->GetMassNo() << ")\n"
			<< std::setw(26) << "Particle Energy:" << genAction->GetParticleEnergy() / MeV << " MeV/nucleon\n";
	}
	else {
		outFS << "e-\n"
			<< std::setw(26) << "Particle Energy:" << genAction->GetParticleEnergy() / MeV << " MeV\n";
	}
	outFS << std::setw(26) << "Beam sigma:" << genAction->GetBeamSigma() / um << " um\n"
		<< std::setw(26) << "Beam sigma error:" << genAction->GetSigErrGauss().sigma() / um << " um\n"
		<< std::setw(26) << "Beam divergence sigma:" << genAction->GetDivSigma() / mrad << " mrad\n"
		<< std::setw(26) << "Number of primaries:" << genAction->GetNoOfParticles() << '\n';
}

void Task_SteppingActionData(std::ofstream& outFS) {
	const SteppingAction* stepAction = dynamic_cast<const SteppingAction*>(G4RunManager::GetRunManager()->GetUserSteppingAction());
	outFS << std::setw(16) << "Verbose level:" << std::to_string(stepAction->GetVerboseLevel()) << '\n';
}

template <>
void ProcessCsvData<int, double, double, double, double>::ReadMePrintAboutCurrentProjectData(std::ofstream& outFS) {
	std::time_t cTimeType = std::chrono::system_clock::to_time_t(o_beginOfRunAction);
	outFS << std::left << std::setw(35) << "The master RunAction began at: " << std::ctime(&cTimeType);
	cTimeType = std::chrono::system_clock::to_time_t(o_endOfRunAction);
	outFS << std::setw(35) << "The master RunAction finished at: " << std::ctime(&cTimeType);
	outFS << std::right << std::setw(61) << std::setfill('=') << '\n' << "DetectorConstruction used values:\n";
	DetectorConstruction* theDetector = DetectorConstruction::GetInstance();
	outFS << std::setw(16) << std::setfill('+') << '\n' << std::setfill(' ') << std::left
		<< std::setw(13) << "Radiator" << "-> " << std::setw(16) << "Thickness:" << theDetector->GetRadiatorThickness() / um << " um\n"
		<< std::setw(16) << ' ' << std::setprecision(1) << std::fixed << std::setw(16) << "Angle:" << theDetector->GetRadiatorAngle() / deg << " deg\n"
		<< std::setw(16) << ' ' << std::setw(16) << std::setprecision(4) << std::defaultfloat << "No of layers:" << std::to_string(theDetector->GetNoOfRadLayers()) << '\n'
		<< std::setw(16) << ' ' << std::setw(16) << "Used material: " << theDetector->GetRadiatorMaterialName() << '\n'
		<< std::setw(16) << std::right << std::setfill('+') << '\n' << std::setfill(' ') << std::left
		<< std::setw(13) << "Detector" << "-> " << std::setw(16) << "Radius:" << theDetector->GetDetectorRadius() / um << " um\n"
		<< std::setw(16) << ' ' << std::setw(16) << std::setprecision(1) << std::fixed << "Angle:" << theDetector->GetDetectorAngle() / deg << " deg\n"
		<< std::setw(16) << ' ' << std::setw(16) << std::setprecision(4) << std::defaultfloat << "Distance:" << theDetector->GetDetectorDistance() / mm << " mm\n"
		<< std::setw(16) << std::right << std::setfill('+') << '\n' << std::setfill(' ') << std::left
		<< std::setw(16) << "Verbose level:" << std::to_string(theDetector->GetVerboseLevel()) << '\n'
		<< std::setw(16) << "Overlap check:";
	if (theDetector->GetCheckOverlap())
		outFS << "true\n";
	else
		outFS << "false\n";
	outFS << std::right << std::setw(61) << std::setfill('=') << '\n' << "PrimaryGeneratorAction used values:\n";
	m_taskGroup.exec([&] {Task_PrintPrimaryGenActionData(outFS); }); //seems like the simplest way to wrap a task
	m_taskGroup.wait();
	const PhysicsList* physList = dynamic_cast<const PhysicsList*>(G4RunManager::GetRunManager()->GetUserPhysicsList());
	outFS << std::right << std::setw(61) << std::setfill('=') << '\n' << "PhysicsList used values:\n" << std::setfill(' ') << std::left
		<< std::setw(29) << "EM used model:" << physList->GetEMPhysicsInUse() << '\n'
		<< std::setw(29) << "Non-default cutValues flag:";
	if (physList->GetUseNonDefaultCuts()) {
		outFS << "true\n"
			<< std::setw(29) << "Radiator gamma cutValue:" << physList->GetRadiatorRangeCuts_gamma() / um << " um\n"
			<< std::setw(29) << "Radiator e- cutValue:" << physList->GetRadiatorRangeCuts_electron() / um << " um\n"
			<< std::setw(29) << "Radiator e+ cutValue:" << physList->GetRadiatorRangeCuts_positron() / um << " um\n"
			<< std::setw(29) << "Radiator proton cutValue:" << physList->GetRadiatorRangeCuts_proton() / um << " um\n";
	}
	else
		outFS << "false\n";
	outFS << std::right << std::setw(61) << std::setfill('=') << '\n' << "SteppingAction used values:\n" << std::setfill(' ') << std::left;
	m_taskGroup.exec([&] {Task_SteppingActionData(outFS); });
	m_taskGroup.wait();
	outFS << std::right << std::setw(61) << std::setfill('=') << '\n' << "MyOpticalParameters<ChRModelIndex> used values:\n" << std::setfill(' ') << std::left;
	const auto myOptParams = MyOpticalParameters<ChRModelIndex>::GetInstance();
	outFS << std::setw(16) << "Verbose level:" << myOptParams->GetVerboseLevel() << '\n'
		<< "Cherenkov material data:\n";
	for (auto& [key, value] : myOptParams->GetChRMatData()) {
		outFS << std::right << std::setw(21) << std::setfill('+') << '\n' << std::setfill(' ') << std::left
			<< std::setw(18) << key->GetName() << "-> " << std::setw(21) << "Material thickness:" << value.m_matThickness / um << " um\n"
			<< std::setw(21) << ' ' << std::setw(21) << std::setprecision(4) << std::fixed << "Max RIndex:" << value.m_maxRIndex << '\n'
			<< std::setw(21) << ' ' << std::setw(21) << std::defaultfloat << "ChR model:" << value.m_forceModel << '\n';
	}
	const CherenkovProcess<ChRModelIndex>* chProc = dynamic_cast<const CherenkovProcess<ChRModelIndex>*>(G4ProcessTable::GetProcessTable()->FindProcess("Cherenkov", "e-"));
	outFS << std::right << std::setw(61) << std::setfill('=') << '\n' << "Registered models in CherenkovProcess<ChRModelIndex>:\n" << std::setfill(' ') << std::left;
	for (auto& [key, value] : chProc->GetChRRegisteredModels()) {
		outFS << std::right << std::setw(21) << std::setfill('+') << '\n' << std::setfill(' ') << std::left
			<< std::setw(18) << key << "-> " << std::setw(24) << "Model name:" << value->GetChRModelName() << '\n'
			<< std::setw(21) << ' ' << std::setw(24) << "Finite thickness flag:";
		if (value->GetFiniteThicknessCondition())
			outFS << "true\n";
		else
			outFS << "false\n";
		outFS << std::setw(21) << ' ' << std::setw(24) << "Energy loss flag:";
		if (value->GetUseModelWithEnergyLoss())
			outFS << "true\n";
		else
			outFS << "false\n";
		outFS << std::setw(21) << ' ' << std::setw(24) << "No of beta steps:" << std::to_string(value->GetNoOfBetaSteps()) << '\n'
			<< std::setw(21) << ' ' << std::setw(24) << "Verbose level:" << std::to_string(value->GetVerboseLevel()) << '\n';
	}
}

//=========public ChR::RunAction:: methods=========
RunAction::RunAction(const EventAction* eAction)
: p_eventAction(eAction),
m_fileExtension{"csv"},
m_fileName{"The_results"},
m_ntName{"ChR_project"} {
	G4RunManager::GetRunManager()->SetPrintProgress(1e6);

	G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
	analysisManager->SetDefaultFileType("root");
	analysisManager->SetNtupleMerging(true);

	analysisManager->CreateNtuple(m_ntName, "The results");
	analysisManager->CreateNtupleIColumn("Event_ID");
	analysisManager->CreateNtupleDColumn("Energy");
	analysisManager->CreateNtupleDColumn("Wavelength");
	analysisManager->CreateNtupleDColumn("x-corrdinate");
	analysisManager->CreateNtupleDColumn("y-coordinate");
	analysisManager->FinishNtuple();
}

RunAction::~RunAction() {
	
}

void RunAction::BeginOfRunAction(const G4Run* aRun) {
	G4AnalysisManager::Instance()->OpenFile(m_fileName + '.' + m_fileExtension);
	if (!isMaster)
		LoadPrimaryGeneratorData();
	else
		o_beginOfRunAction = std::chrono::system_clock::now();
}

void RunAction::EndOfRunAction(const G4Run* aRun) {
	G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
	analysisManager->Write();
	analysisManager->CloseFile();
	if (!isMaster) {
		if (DetectorConstruction::GetInstance()->GetNoOfRadLayers() > 1) {
			std::lock_guard lck(o_RunManagerMutex);
			const std::vector<EventAction::LayerData>& layerData = p_eventAction->GetLayerDataVec();
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
				G4Exception("RunAction::EndOfRunAction", "FE1019", FatalException, "Various worker threads with various sizes of layerDataVectors!\n");
			}
		}
	}
	else {
		o_endOfRunAction = std::chrono::system_clock::now();
		if (DetectorConstruction::GetInstance()->GetNoOfRadLayers() > 1) {
			//save o_LayerDataVec
			std::ofstream runOFS;
			runOFS.open("LostEPerLayer.csv", std::ios::out | std::ios::trunc);
			if (!runOFS)
				G4Exception("RunAction::EndOfRunAction", "WE1010", JustWarning, "Didn't manage to open \"LostEPerLayer.csv\"");
			else {
				for (auto& forPrinting : o_LayerDataVec)
					runOFS << forPrinting.m_count << ',' << forPrinting.m_lostE / MeV << ',' << forPrinting.m_stepLng / um << '\n';
				runOFS.close();
			}
			//end of saving o_LayerDataVec
			//============================================
		}
		//do initial processing of raw data if csv is in use
		if (m_fileExtension == "csv") {
			TimeBench<std::chrono::microseconds> timeBNCH{"RunAction::EndOfRunAction - processCSV"};
			ProcessCsvData<int, double, double, double, double> processCsv{m_fileName, m_fileExtension, m_ntName};
			processCsv.MoveAFileToFinalDestination("LostEPerLayer.csv");
			processCsv.Proces_N_D_Data<2>(std::vector<double>{1., 2., 3.}, "Eff_PeakWaveLng", "efficiency.csv");
			processCsv.Proces_N_D_Data<2>(std::vector<double>{1., 2., 3.}, "PeakWaveLng");
			processCsv.Proces_N_D_Data<3, 4>(std::vector<double>{3.}, "DetDist");
		}
	}
}

//=========private ChR::RunAction:: methods=========
void RunAction::LoadPrimaryGeneratorData() {
	PrimaryGeneratorAction* myGenAction = PrimaryGeneratorAction::GetInstance();
	G4ParticleGun* theGun = myGenAction->GetParticleGun();
	if (myGenAction->GetMassNo() != 0 && myGenAction->GetAtomicNo() != 0) {
		theGun->SetParticleDefinition(G4IonTable::GetIonTable()->GetIon(myGenAction->GetAtomicNo(), myGenAction->GetMassNo(), 0.));
		theGun->SetParticleCharge(myGenAction->GetAtomicNo() * eplus);
		theGun->SetParticleEnergy(myGenAction->GetMassNo() * myGenAction->GetParticleEnergy());
	}
	else {
		theGun->SetParticleDefinition(G4ParticleTable::GetParticleTable()->FindParticle("e-"));
		theGun->SetParticleEnergy(myGenAction->GetParticleEnergy());
	}
	theGun->SetNumberOfParticles(myGenAction->GetNoOfParticles());
	DetectorConstruction* theDetector = DetectorConstruction::GetInstance();
	myGenAction->SetDistanceZ(theDetector->GetRadiatorThickness() / std::cos(theDetector->GetRadiatorAngle()) +
		5 * myGenAction->GetBeamSigma() * std::tan(theDetector->GetRadiatorAngle()));
}

endChR