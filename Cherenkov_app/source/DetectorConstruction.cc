//##########################################
//#######        VERSION 1.1.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

// user headers
#include "DetectorConstruction.hh"
// G4 headers (from ChR_process_lib)
#include "G4ExtraOpticalParameters.hh"
// ...
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4VisAttributes.hh"
#include "G4Region.hh"
#include "G4RunManager.hh"

beginChR

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//=========public ChR::DetectorConstruction:: methods=========
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{
  delete fDetConstructionMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  LoadWorld();
  LoadRadiator();
#ifdef standardRun
  LoadDetector();
#endif // standardRun
  return fWorldPhys;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//=========private ChR::DetectorConstruction:: methods=========
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction()
: fVisAttrHide(),
  fDetConstructionMessenger(new DetectorConstruction_Messenger{ this }),
  fRadiatorMaterialName("G4_SILICON_DIOXIDE"),
  fRadiatorAngle(22.0 * deg),
  fRadiatorThickness(100. * um),
  fDetectorRadius(35. * um),
  fDetectorAngle(59.5 * deg),
  fDetectorDistance(37.6 * cm)
{
  LoadMaterials();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::LoadMaterials()
{
  G4NistManager* nist = G4NistManager::Instance();
  G4double airPres = nist->FindOrBuildMaterial("G4_AIR")->GetPressure();
  G4Material* quartzMat = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE"); // as quartz (not completely correct, but good enough)
  G4Material* diamondMat = nist->BuildMaterialWithNewDensity("Diamond", "G4_C", 3.5 * g / cm3);
  G4Material* vacuumMat = nist->BuildMaterialWithNewDensity("Low_density_air", "G4_AIR", airPres * 1.e-9);
  // just need some low value pressure to minimize interaction, the value is not important

  // adding material properties
  HelperToBuildMatPropTable obj{}; // could have used namespace instead of a struct
  obj.FillMatPropData(vacuumMat, "RINDEX", [](G4double) {return 1.; }, 1.458637 * eV, 4.13280 * eV, 2);
  obj.FillMatPropData(quartzMat, "ABSLENGTH", [](G4double) {return 0.5 * m; }, 1.458637 * eV, 4.13280 * eV, 2);
  obj.FillMatPropData(diamondMat, "ABSLENGTH", [](G4double) {return 0.5 * m; }, 1.458637 * eV, 4.13280 * eV, 2);
#ifdef standardRun
  obj.FillMatPropertiesVectors("refractive_index_data.txt", quartzMat, EnergyValue::Wavelength, 1. * um, '\t', "Energy", "RINDEX");
#else
  G4Material* fakeQuartzMat = nist->BuildMaterialWithNewDensity("fake_quartz", "G4_SILICON_DIOXIDE", quartzMat->GetDensity());
  obj.FillMatPropertiesVectors("refractive_index_data.txt", fakeQuartzMat, EnergyValue::Energy, 1. * eV, '\t', "Energy", "RINDEX");
#endif // standardRun
  obj.FillMatPropertiesVectors("refractive_index_data.txt", diamondMat, EnergyValue::Wavelength, 1. * um, '\t', "Energy", "RINDEX");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::LoadWorld()
{
  G4Box* worldSolid = new G4Box{ "worldSolid", 0.5 * m, 0.5 * m, 0.5 * m };
  G4LogicalVolume* worldLogic = new G4LogicalVolume{ worldSolid, G4Material::GetMaterial("Low_density_air"), "worldLogic" };
  fWorldPhys = new G4PVPlacement{ nullptr, G4ThreeVector(0.,0.,0.), worldLogic, "worldPhys", nullptr, false, 0, true };
  fVisAttrHide = std::make_unique<G4VisAttributes>(false);
  worldLogic->SetVisAttributes(fVisAttrHide.get());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::LoadRadiator()
{
#ifndef standardRun
  fRadiatorMaterialName = "fake_quartz";
#endif // !standardRun
  G4Tubs* radiatorSolid = new G4Tubs{ "radiatorSolid", 0., 3. * cm, fRadiatorThickness, 0., CLHEP::twopi };
  G4LogicalVolume* radiatorLogic = new G4LogicalVolume{ radiatorSolid, G4Material::GetMaterial(fRadiatorMaterialName), "radiatorLogic" };
  if (dynamic_cast<const PhysicsList*>(G4RunManager::GetRunManager()->GetUserPhysicsList())->GetOpticalPhysicsInUse() == UseOptical::G4OpticalPhysics_option2) {
    auto extraOptParams = G4ExtraOpticalParameters::GetInstance();
    extraOptParams->AddNewChRMatData(radiatorLogic, G4CherenkovMatData{ 1 });
  }
  G4Rotate3D radRot{ fRadiatorAngle, G4ThreeVector{ -1., 0., 0. } };
  G4Transform3D radTransform{ radRot };
  G4VPhysicalVolume* radiatorPhys = new G4PVPlacement{ radTransform, "radiatorPhys", radiatorLogic, fWorldPhys, false, 0, true };
  G4Region* radiatorRegion = new G4Region("radiatorRegion");
  radiatorRegion->AddRootLogicalVolume(radiatorLogic);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::LoadDetector() const
{
  constexpr G4double detThickness = 0.5 * cm; // half-thickness
  // it's good to take some greater number, as 0.5 cm, to actually understand where it is (in gui mode)
  G4Tubs* detectorSolid = new G4Tubs{ "detectorSolid", 0., fDetectorRadius, detThickness, 0., 360. * deg };
  //
  G4LogicalVolume* p_detectorLogic = new G4LogicalVolume{ detectorSolid, G4Material::GetMaterial("G4_AIR"), "detectorLogic" };
  /*
  for detectorSolid, the material is not important because efficiency of 100% is considered.
  Only later in data processing efficiency can be included
  NOTE: if the material doesn't have MaterialProperties defined (e.g., G4_Air), opticalphotons won't be able to
  enter the volume and they will be killed! However, G4StepStatus::fGeomBoundary can be used to extract the
  information (that's done in SteppingAction::UserSteppingAction in this project)... That's because a very basic
  geometry is considered here. SensitiveDetectors won't be able to detect opticalphotons if there's no RINDEX defined.
  That means, for example, using "Low_density_air" instead of "G4_AIR" would solve that problem
  */
  G4Rotate3D detRot{ fDetectorAngle, G4ThreeVector{ -1.,0.,0. } };
  G4Translate3D detTranslate{ G4ThreeVector{ 0.,0.,fDetectorDistance + detThickness } };
  G4Transform3D detTransform{ detRot * detTranslate }; // first rotate and then translate in the new coordinate system
  new G4PVPlacement{ detTransform, "detectorPhys", p_detectorLogic, fWorldPhys, false, 0, true };
}

endChR