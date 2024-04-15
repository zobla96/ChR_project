#include "DetectorConstruction_Messenger.hpp"

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

beginChR

//=========public ChR::DetectorConstruction_Messenger:: methods=========
DetectorConstruction_Messenger::DetectorConstruction_Messenger(DetectorConstruction* detConstruct)
:p_detectorConstruction(detConstruct) {
	p_detectorMessengerDir = new G4UIdirectory{ "/ChR_project/DetectorConsturction/" };
	p_detectorMessengerDir->SetGuidance("All commands related to geometry");

	p_checkOverlap = new G4UIcmdWithABool{ "/ChR_project/DetectorConsturction/checkOverlap", this };
	p_checkOverlap->SetGuidance("A bool to check whether some geometry elements overlap.");
	p_checkOverlap->SetGuidance("Set \"false\" if you don't want to check... \"true\" is by default");
	p_checkOverlap->SetParameterName("checkCondition", true);
	p_checkOverlap->SetDefaultValue(true);
	p_checkOverlap->AvailableForStates(G4State_PreInit);

	p_noOfRadLayers = new G4UIcmdWithAnInteger{ "/ChR_project/DetectorConsturction/noOfRadLayers", this };
	p_noOfRadLayers->SetGuidance("Used to change the number of layers of the radiator.");
	p_noOfRadLayers->SetGuidance("In this project, it is used to determine the lost energy as a function of the particle's penetration depth.");
	p_noOfRadLayers->SetGuidance("All such data are collected using the G4StepStatus::fGeomBoundary condition.");
	p_noOfRadLayers->SetGuidance("As long as this value is less than 2, the energy-loss data are not collected!");
	p_noOfRadLayers->SetParameterName("noOfRadiatorLayers", true);
	p_noOfRadLayers->SetDefaultValue(1);
	p_noOfRadLayers->SetRange("noOfRadiatorLayers>=0 && noOfRadiatorLayers<256");
	p_noOfRadLayers->AvailableForStates(G4State_PreInit);

	p_radiatorMaterial = new G4UIcmdWithAString{ "/ChR_project/DetectorConsturction/radiatorMaterial", this };
	p_radiatorMaterial->SetGuidance("Used to change the radiator material (based on material name).");
	p_radiatorMaterial->SetGuidance("'Diamond' should be used for GenericIons, while 'G4_SILICON_DIOXIDE' should be used for electrons");
	p_radiatorMaterial->SetParameterName("materialName", true);
	p_radiatorMaterial->SetDefaultValue("G4_SILICON_DIOXIDE");
	p_radiatorMaterial->SetCandidates("G4_SILICON_DIOXIDE Diamond");
	p_radiatorMaterial->AvailableForStates(G4State_PreInit);

	p_verboseLevel = new G4UIcmdWithAnInteger{ "/ChR_project/DetectorConsturction/verboseLevel", this };
	p_verboseLevel->SetGuidance("Used to change verboseLevel of the DetectorConstruction class.");
	p_verboseLevel->SetGuidance("NOTE: it's useless right now as I still didn't use it anywhere.");
	p_verboseLevel->SetParameterName("verboseLevel", true);
	p_verboseLevel->SetDefaultValue(1);
	p_verboseLevel->SetRange("verboseLevel>=0 && verboseLevel<256");
	p_verboseLevel->AvailableForStates(G4State_PreInit);

	//for the following, I might add the range in the future... for now no need
	p_radiatorAngle = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/DetectorConsturction/radiatorAngle", this };
	p_radiatorAngle->SetGuidance("Used to change the radiator angle.");
	p_radiatorAngle->SetGuidance("By changing the radiator angle, one can extract Cherenkov radiation at different angles!");
	p_radiatorAngle->SetParameterName("radiatorAngle", false);
	//p_radiatorAngle->SetUnitCandidates("deg rad");
	p_radiatorAngle->SetDefaultUnit("deg");
	p_radiatorAngle->SetDefaultValue(23.0);
	p_radiatorAngle->AvailableForStates(G4State_PreInit);

	p_radiatorThickness = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/DetectorConsturction/radiatorThickness", this };
	p_radiatorThickness->SetGuidance("Used to change the radiator thickness (half-thickness).");
	p_radiatorThickness->SetParameterName("radiatorThickness", false);
	//p_radiatorThickness->SetUnitCandidates("um, mm");
	p_radiatorThickness->SetDefaultUnit("um");
	p_radiatorThickness->SetDefaultValue(100.);
	p_radiatorThickness->AvailableForStates(G4State_PreInit);

	p_detectorRadius = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/DetectorConsturction/detectorRadius", this };
	p_detectorRadius->SetGuidance("Used to change the detector radius (outer value).");
	p_detectorRadius->SetGuidance("It can significantly change the detection efficiency, but increase the peak width as well!");
	p_detectorRadius->SetParameterName("detectorRadius", false);
	//p_detectorRadius->SetUnitCandidates("um, mm");
	p_detectorRadius->SetDefaultUnit("um");
	p_detectorRadius->SetDefaultValue(35.);
	p_detectorRadius->AvailableForStates(G4State_PreInit);

	p_detectorAngle = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/DetectorConsturction/detectorAngle", this };
	p_detectorAngle->SetGuidance("Used to change the detector angle which determines the detector location.");
	p_detectorAngle->SetGuidance("By changing the radiator angle, one can extract Cherenkov radiation at different angles!");
	p_detectorAngle->SetParameterName("detectorAngle", false);
	//p_detectorAngle->SetUnitCandidates("deg rad");
	p_detectorAngle->SetDefaultUnit("deg");
	p_detectorAngle->SetDefaultValue(59.5);
	p_detectorAngle->AvailableForStates(G4State_PreInit);

	p_detectorDistance = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/DetectorConsturction/detectorDistance", this };
	p_detectorDistance->SetGuidance("Used to change the detector distance from the center of the radiator (worldVolume as well).");
	p_detectorDistance->SetGuidance("It can significantly change the detection efficiency! The current default value is also the experimental value!");
	p_detectorDistance->SetParameterName("detectorDistance", false);
	//p_detectorDistance->SetUnitCandidates("um, mm");
	p_detectorDistance->SetDefaultUnit("cm");
	p_detectorDistance->SetDefaultValue(37.6);
	p_detectorDistance->AvailableForStates(G4State_PreInit);
}

DetectorConstruction_Messenger::~DetectorConstruction_Messenger() {
	delete p_detectorMessengerDir;
	delete p_checkOverlap;
	delete p_noOfRadLayers;
	delete p_radiatorMaterial;
	delete p_verboseLevel;
	delete p_radiatorAngle;
	delete p_radiatorThickness;
	delete p_detectorRadius;
	delete p_detectorAngle;
	delete p_detectorDistance;
}

void DetectorConstruction_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr) {
	if (uiCmd == p_checkOverlap) {
		p_detectorConstruction->SetCheckOverlap(p_checkOverlap->ConvertToBool(aStr));
		std::cout << "The detector's m_checkOverlap has been set to " << aStr << '\n';
	}
	else if (uiCmd == p_noOfRadLayers) {
		p_detectorConstruction->SetNoOfRadLayers((const unsigned char)p_noOfRadLayers->ConvertToInt(aStr));
		std::cout << "The detector's m_noOfRadLayers has been set to " << aStr << '\n';
	}
	else if (uiCmd == p_radiatorMaterial) {
		p_detectorConstruction->SetRadiatorMaterialName(aStr);
		std::cout << "The detector's m_radiatorMaterial has been set to " << aStr << '\n';
	}
	else if (uiCmd == p_verboseLevel) {
		p_detectorConstruction->SetVerboseLevel((const unsigned char)p_noOfRadLayers->ConvertToInt(aStr));
		std::cout << "The detector's m_verbose has been set to " << aStr << '\n';
	}
	else if (uiCmd == p_radiatorAngle) {
		p_detectorConstruction->SetRadiatorAngle(p_radiatorAngle->GetNewDoubleValue(aStr));
		std::cout << "The detector's m_radiatorAngle has been set to " << aStr << '\n';
	}
	else if (uiCmd == p_radiatorThickness) {
		p_detectorConstruction->SetRadiatorThickness(p_radiatorThickness->GetNewDoubleValue(aStr));
		std::cout << "The detector's m_radiatorThickness has been set to " << aStr << '\n';
	}
	else if (uiCmd == p_detectorRadius) {
		p_detectorConstruction->SetDetectorRadius(p_detectorRadius->GetNewDoubleValue(aStr));
		std::cout << "The detector's m_detectorRadius has been set to " << aStr << '\n';
	}
	else if (uiCmd == p_detectorAngle) {
		p_detectorConstruction->SetDetectorAngle(p_detectorAngle->GetNewDoubleValue(aStr));
		std::cout << "The detector's m_detectorAngle has been set to " << aStr << '\n';
	}
	else if (uiCmd == p_detectorDistance) {
		p_detectorConstruction->SetDetectorDistance(p_detectorDistance->GetNewDoubleValue(aStr));
		std::cout << "The detector's m_detectorDistance has been set to " << aStr << '\n';
	}
	else
		G4Exception("DetectorConstruction_Messenger::SetNewValue", "WE1013", JustWarning, "Command not found!\n");
}

endChR