//##########################################
//#######        VERSION 1.1.0       #######
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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//=========public ChR::DetectorConstruction_Messenger:: methods=========
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction_Messenger::DetectorConstruction_Messenger(DetectorConstruction* detConstruct)
: fDetectorConstruction(detConstruct),
  fDetectorMessengerDir(new G4UIdirectory{ "/ChR_project/DetectorConstruction/" }),
  fRadiatorMaterial(new G4UIcmdWithAString{ "/ChR_project/DetectorConstruction/radiatorMaterial", this }),
  fRadiatorAngle(new G4UIcmdWithADoubleAndUnit{ "/ChR_project/DetectorConstruction/radiatorAngle", this }),
  fRadiatorThickness(new G4UIcmdWithADoubleAndUnit{ "/ChR_project/DetectorConstruction/radiatorThickness", this }),
  fDetectorRadius(new G4UIcmdWithADoubleAndUnit{ "/ChR_project/DetectorConstruction/detectorRadius", this }),
  fDetectorAngle(new G4UIcmdWithADoubleAndUnit{ "/ChR_project/DetectorConstruction/detectorAngle", this }),
  fDetectorDistance(new G4UIcmdWithADoubleAndUnit{ "/ChR_project/DetectorConstruction/detectorDistance", this })
{

  fDetectorMessengerDir->SetGuidance("All commands related to geometry");

  fRadiatorMaterial->SetGuidance("Used to change the radiator material (based on material name).");
  fRadiatorMaterial->SetGuidance("'Diamond' should be used for GenericIons of low energy, while 'G4_SILICON_DIOXIDE' should be used for electrons");
  fRadiatorMaterial->SetParameterName("materialName", true);
  fRadiatorMaterial->SetDefaultValue("G4_SILICON_DIOXIDE");
  fRadiatorMaterial->SetCandidates("G4_SILICON_DIOXIDE Diamond"); // in non-standard runs fake_quartz is used unless selected otherwise
  fRadiatorMaterial->SetToBeBroadcasted(false);
  fRadiatorMaterial->AvailableForStates(G4State_PreInit);

  fRadiatorAngle->SetGuidance("Used to change the radiator angle.");
  fRadiatorAngle->SetGuidance("By changing the radiator angle, one can extract Cherenkov radiation at different angles!");
  fRadiatorAngle->SetParameterName("radiatorAngle", false);
  //fRadiatorAngle->SetUnitCandidates("deg rad");
  fRadiatorAngle->SetDefaultUnit("deg");
  fRadiatorAngle->SetRange("radiatorAngle>=0. && radiatorAngle<90.");
  fRadiatorAngle->SetToBeBroadcasted(false);
  fRadiatorAngle->AvailableForStates(G4State_PreInit);

  fRadiatorThickness->SetGuidance("Used to change the radiator thickness (half-thickness).");
  fRadiatorThickness->SetParameterName("radiatorThickness", false);
  //fRadiatorThickness->SetUnitCandidates("um, mm");
  fRadiatorThickness->SetDefaultUnit("um");
  fRadiatorThickness->SetToBeBroadcasted(false);
  fRadiatorThickness->AvailableForStates(G4State_PreInit);

  fDetectorRadius->SetGuidance("Used to change the detector radius (outer value).");
  fDetectorRadius->SetGuidance("NOTE: It can significantly change the detection efficiency (angular acceptance)!");
  fDetectorRadius->SetParameterName("detectorRadius", false);
  //fDetectorRadius->SetUnitCandidates("um, mm");
  fDetectorRadius->SetDefaultUnit("um");
  fDetectorRadius->SetToBeBroadcasted(false);
  fDetectorRadius->AvailableForStates(G4State_PreInit);

  fDetectorAngle->SetGuidance("Used to change the detector angle relative to the radiator.");
  fDetectorAngle->SetGuidance("By changing the detector angle, one can observe Cherenkov spectral lines on different wavelengths!");
  fDetectorAngle->SetParameterName("detectorAngle", false);
  //fDetectorAngle->SetUnitCandidates("deg rad");
  fDetectorAngle->SetDefaultUnit("deg");
  fDetectorAngle->SetToBeBroadcasted(false);
  fDetectorAngle->AvailableForStates(G4State_PreInit);

  fDetectorDistance->SetGuidance("Used to change the detector distance from the center of the radiator (worldVolume as well).");
  fDetectorDistance->SetGuidance("It can significantly change the detection efficiency! The current default value is also the experimental value!");
  fDetectorDistance->SetParameterName("detectorDistance", false);
  //fDetectorDistance->SetUnitCandidates("cm, mm");
  fDetectorDistance->SetDefaultUnit("cm");
  fDetectorDistance->SetToBeBroadcasted(false);
  fDetectorDistance->AvailableForStates(G4State_PreInit);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction_Messenger::~DetectorConstruction_Messenger()
{
  delete fDetectorMessengerDir;
  delete fRadiatorMaterial;
  delete fRadiatorAngle;
  delete fRadiatorThickness;
  delete fDetectorRadius;
  delete fDetectorAngle;
  delete fDetectorDistance;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction_Messenger::SetNewValue(G4UIcommand* uiCmd, G4String aStr)
{
  if (uiCmd == fRadiatorMaterial)
    fDetectorConstruction->SetRadiatorMaterialName(aStr);
  else if (uiCmd == fRadiatorAngle)
    fDetectorConstruction->SetRadiatorAngle(fRadiatorAngle->GetNewDoubleValue(aStr));
  else if (uiCmd == fRadiatorThickness)
    fDetectorConstruction->SetRadiatorThickness(fRadiatorThickness->GetNewDoubleValue(aStr));
  else if (uiCmd == fDetectorRadius)
    fDetectorConstruction->SetDetectorRadius(fDetectorRadius->GetNewDoubleValue(aStr));
  else if (uiCmd == fDetectorAngle)
    fDetectorConstruction->SetDetectorAngle(fDetectorAngle->GetNewDoubleValue(aStr));
  else if (uiCmd == fDetectorDistance)
    fDetectorConstruction->SetDetectorDistance(fDetectorDistance->GetNewDoubleValue(aStr));
  else // should never happen
    G4Exception("DetectorConstruction_Messenger::SetNewValue", "FE_DetMessenger01", FatalException, "Command not found!\n");
}

endChR