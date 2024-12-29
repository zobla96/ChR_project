//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

// user headers
#include "DetectorConstruction_Messenger.hh"
// G4 headers
#include "G4UIdirectory.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithAString.hh"

beginChR

//=========public ChR::DetectorConstruction_Messenger:: methods=========

DetectorConstruction_Messenger::DetectorConstruction_Messenger(DetectorConstruction* detConstruct)
:p_detectorConstruction(detConstruct) {
	p_detectorMessengerDir = new G4UIdirectory{ "/ChR_project/DetectorConstruction/" };
	p_detectorMessengerDir->SetGuidance("All commands related to geometry");

	p_radiatorMaterial = new G4UIcmdWithAString{ "/ChR_project/DetectorConstruction/radiatorMaterial", this };
	p_radiatorMaterial->SetGuidance("Used to change the radiator material (based on material name).");
	p_radiatorMaterial->SetGuidance("'Diamond' should be used for GenericIons of low energy, while 'G4_SILICON_DIOXIDE' should be used for electrons");
	p_radiatorMaterial->SetParameterName("materialName", true);
	p_radiatorMaterial->SetDefaultValue("G4_SILICON_DIOXIDE");
	p_radiatorMaterial->SetCandidates("G4_SILICON_DIOXIDE Diamond"); // in non-standard runs fake_quartz is used unless selected otherwise
	p_radiatorMaterial->SetToBeBroadcasted(false);
	p_radiatorMaterial->AvailableForStates(G4State_PreInit);

	p_radiatorAngle = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/DetectorConstruction/radiatorAngle", this };
	p_radiatorAngle->SetGuidance("Used to change the radiator angle.");
	p_radiatorAngle->SetGuidance("By changing the radiator angle, one can extract Cherenkov radiation at different angles!");
	p_radiatorAngle->SetParameterName("radiatorAngle", false);
	//p_radiatorAngle->SetUnitCandidates("deg rad");
	p_radiatorAngle->SetDefaultUnit("deg");
	p_radiatorAngle->SetRange("radiatorAngle>=0. && radiatorAngle<90.");
	p_radiatorAngle->SetToBeBroadcasted(false);
	p_radiatorAngle->AvailableForStates(G4State_PreInit);

	p_radiatorThickness = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/DetectorConstruction/radiatorThickness", this };
	p_radiatorThickness->SetGuidance("Used to change the radiator thickness (half-thickness).");
	p_radiatorThickness->SetParameterName("radiatorThickness", false);
	//p_radiatorThickness->SetUnitCandidates("um, mm");
	p_radiatorThickness->SetDefaultUnit("um");
	p_radiatorThickness->SetToBeBroadcasted(false);
	p_radiatorThickness->AvailableForStates(G4State_PreInit);

	p_detectorRadius = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/DetectorConstruction/detectorRadius", this };
	p_detectorRadius->SetGuidance("Used to change the detector radius (outer value).");
	p_detectorRadius->SetGuidance("NOTE: It can significantly change the detection efficiency (angular acceptance)!");
	p_detectorRadius->SetParameterName("detectorRadius", false);
	//p_detectorRadius->SetUnitCandidates("um, mm");
	p_detectorRadius->SetDefaultUnit("um");
	p_detectorRadius->SetToBeBroadcasted(false);
	p_detectorRadius->AvailableForStates(G4State_PreInit);

	p_detectorAngle = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/DetectorConstruction/detectorAngle", this };
	p_detectorAngle->SetGuidance("Used to change the detector angle relative to the radiator.");
	p_detectorAngle->SetGuidance("By changing the detector angle, one can observe Cherenkov spectral lines on different wavelengths!");
	p_detectorAngle->SetParameterName("detectorAngle", false);
	//p_detectorAngle->SetUnitCandidates("deg rad");
	p_detectorAngle->SetDefaultUnit("deg");
	p_detectorAngle->SetToBeBroadcasted(false);
	p_detectorAngle->AvailableForStates(G4State_PreInit);

	p_detectorDistance = new G4UIcmdWithADoubleAndUnit{ "/ChR_project/DetectorConstruction/detectorDistance", this };
	p_detectorDistance->SetGuidance("Used to change the detector distance from the center of the radiator (worldVolume as well).");
	p_detectorDistance->SetGuidance("It can significantly change the detection efficiency! The current default value is also the experimental value!");
	p_detectorDistance->SetParameterName("detectorDistance", false);
	//p_detectorDistance->SetUnitCandidates("cm, mm");
	p_detectorDistance->SetDefaultUnit("cm");
	p_detectorDistance->SetToBeBroadcasted(false);
	p_detectorDistance->AvailableForStates(G4State_PreInit);
}

DetectorConstruction_Messenger::~DetectorConstruction_Messenger() {
	delete p_detectorMessengerDir;
	delete p_radiatorMaterial;
	delete p_radiatorAngle;
	delete p_radiatorThickness;
	delete p_detectorRadius;
	delete p_detectorAngle;
	delete p_detectorDistance;
}

void DetectorConstruction_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr) {
	if (uiCmd == p_radiatorMaterial)
		p_detectorConstruction->SetRadiatorMaterialName(aStr);
	else if (uiCmd == p_radiatorAngle)
		p_detectorConstruction->SetRadiatorAngle(p_radiatorAngle->GetNewDoubleValue(aStr));
	else if (uiCmd == p_radiatorThickness)
		p_detectorConstruction->SetRadiatorThickness(p_radiatorThickness->GetNewDoubleValue(aStr));
	else if (uiCmd == p_detectorRadius)
		p_detectorConstruction->SetDetectorRadius(p_detectorRadius->GetNewDoubleValue(aStr));
	else if (uiCmd == p_detectorAngle)
		p_detectorConstruction->SetDetectorAngle(p_detectorAngle->GetNewDoubleValue(aStr));
	else if (uiCmd == p_detectorDistance)
		p_detectorConstruction->SetDetectorDistance(p_detectorDistance->GetNewDoubleValue(aStr));
	else // should never happen
		G4Exception("DetectorConstruction_Messenger::SetNewValue", "FE_DetMessenger01", FatalException, "Command not found!\n");
}

endChR