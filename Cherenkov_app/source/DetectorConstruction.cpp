//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#include "DetectorConstruction.hpp"

beginChR

//=========public ChR::DetectorConstruction:: methods=========

DetectorConstruction::~DetectorConstruction() {
	delete p_detConstructionMessenger;
}

G4VPhysicalVolume* DetectorConstruction::Construct() {
	LoadWorld();
	LoadRadiator();
#ifdef standardRun
	LoadDetector();
#endif // standardRun
	return p_worldPhys;
}

//=========private ChR::DetectorConstruction:: methods=========

DetectorConstruction::DetectorConstruction()
: m_radiatorMaterialName("G4_SILICON_DIOXIDE"), 
m_radiatorAngle(22.0_deg),
m_radiatorThickness(100._um),
m_detectorRadius(35._um),
m_detectorAngle(59.5_deg),
m_detectorDistance(37.6_cm),
m_noOfRadLayers(1),
m_verbose(0),
m_checkOverlap(true) {
	p_detConstructionMessenger = new DetectorConstruction_Messenger{ this };
	LoadMaterials();
}

void DetectorConstruction::LoadMaterials() {
	G4NistManager* nist = G4NistManager::Instance();
	double airPres = nist->FindOrBuildMaterial("G4_AIR")->GetPressure();
	G4Material* quartzMat = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE"); //we use this one as quartz (not completely correct, but good enough)
	G4Material* diamondMat = nist->BuildMaterialWithNewDensity("Diamond", "G4_C", 3.5 * g / cm3);
	G4Material* vacuumMat = nist->BuildMaterialWithNewDensity("Low_density_air", "G4_AIR", airPres * 1.e-9);
	//just need some low value pressure to minimize interaction, the value is not important

	//adding material properties
	HelperToBuildMatPropTable obj{}; //could have used namespace instead of a struct
	obj.FillMatPropData(vacuumMat, "RINDEX", [](double) {return 1.; }, 1.458637_eV, 4.13280_eV, 2);
	obj.FillMatPropData(quartzMat, "ABSLENGTH", [](double) {return 0.5_m; }, 1.458637_eV, 4.13280_eV, 2);
	obj.FillMatPropData(diamondMat, "ABSLENGTH", [](double) {return 0.5_m; }, 1.458637_eV, 4.13280_eV, 2);
#ifdef standardRun
	obj.FillMatPropertiesVectors("refractive_index_data.txt", quartzMat, EnergyValue::Wavelength, 1._um, '\t', "Energy", "RINDEX");
#else
	G4Material* fakeQuartzMat = nist->BuildMaterialWithNewDensity("fake_quartz", "G4_SILICON_DIOXIDE", quartzMat->GetDensity());
	obj.FillMatPropertiesVectors("refractive_index_data.txt", fakeQuartzMat, EnergyValue::Energy, 1._eV, '\t', "Energy", "RINDEX");
#endif // standardRun
	obj.FillMatPropertiesVectors("refractive_index_data.txt", diamondMat, EnergyValue::Wavelength, 1._um, '\t', "Energy", "RINDEX");
}

void DetectorConstruction::LoadWorld() {
	G4Box* worldSolid = new G4Box{ "worldSolid", 0.5_m, 0.5_m, 0.5_m };
	G4LogicalVolume* worldLogic = new G4LogicalVolume{ worldSolid, G4Material::GetMaterial("Low_density_air"), "worldLogic" };
	p_worldPhys = new G4PVPlacement{ nullptr, G4ThreeVector(0.,0.,0.), worldLogic, "worldPhys", nullptr, false, 0, m_checkOverlap };
	m_visAttrHide = std::make_unique<G4VisAttributes>(false);
	worldLogic->SetVisAttributes(m_visAttrHide.get());
}

void DetectorConstruction::LoadRadiator() {
#ifndef standardRun
	m_radiatorMaterialName = "fake_quartz";
#endif // !standardRun
	G4Tubs* radiatorSolid = new G4Tubs{ "radiatorSolid", 0., 3._cm, m_radiatorThickness, 0., 360._deg };
	G4LogicalVolume* radiatorLogic = new G4LogicalVolume{ radiatorSolid, G4Material::GetMaterial(m_radiatorMaterialName), "radiatorLogic" };
	if (dynamic_cast<const PhysicsList*>(G4RunManager::GetRunManager()->GetUserPhysicsList())->GetPhysics("OpticalPhysics_op2")) {
		auto extraOptParams = G4ExtraOpticalParameters::GetInstance();
		extraOptParams->AddNewChRMatData(radiatorLogic, G4CherenkovMatData{ 1 });
	}
	//in case we divide the radiator into layers - an envelope
	//otherwise a radiator
	G4Rotate3D radRot{ m_radiatorAngle, G4ThreeVector{ -1., 0., 0. } };
	G4Transform3D radTransform{ radRot };
	G4VPhysicalVolume* radiatorPhys = new G4PVPlacement{ radTransform, "radiatorPhys", radiatorLogic, p_worldPhys, false, 0, m_checkOverlap };
	G4Region* radiatorRegion = new G4Region("radiatorRegion");
	radiatorRegion->AddRootLogicalVolume(radiatorLogic);
	//now dividing for needs of layered detector
	if (m_noOfRadLayers <= 1)
		return;
	//layerThickness -> half-thickness
	double layerThickness = m_radiatorThickness / m_noOfRadLayers;
	G4Tubs* radLayerSolid = new G4Tubs{ "radiatorLayerSolid", 0., 3._cm, layerThickness, 0., 360._deg };
	G4LogicalVolume* radLayerLogic = new G4LogicalVolume{ radLayerSolid, G4Material::GetMaterial(m_radiatorMaterialName), "radiatorLayerLogic" };
	radLayerLogic->SetVisAttributes(m_visAttrHide.get());
	if (dynamic_cast<const PhysicsList*>(G4RunManager::GetRunManager()->GetUserPhysicsList())->GetPhysics("OpticalPhysics_op2")) {
		auto extraOptParams = G4ExtraOpticalParameters::GetInstance();
		extraOptParams->AddNewChRMatData(radLayerLogic, G4CherenkovMatData{ 1 });
	}
	for (unsigned char i = 0; i < m_noOfRadLayers; i++) {
		double zCoord = -m_radiatorThickness + (2 * i + 1) * layerThickness;
		new G4PVPlacement{ nullptr, G4ThreeVector{ 0.,0.,zCoord }, "radiatorLayerPhys", radLayerLogic, radiatorPhys, false, i, m_checkOverlap };
	}
}

void DetectorConstruction::LoadDetector() const {
	constexpr double detThickness = 0.5_cm; //half-thickness
	//it's good to take some greater number, as 0.5 cm, to actually understand where it is (in gui mode)
	G4Tubs* detectorSolid = new G4Tubs{ "detectorSolid", 0., m_detectorRadius, detThickness, 0., 360._deg };
	//
	G4LogicalVolume* p_detectorLogic = new G4LogicalVolume{ detectorSolid, G4Material::GetMaterial("G4_AIR"), "detectorLogic" };
	/*
	for detectorSolid, the material is not important because I consider efficiency of 100%.
	Only later in data processing I include efficiency
	NOTE: if the material doesn't have MaterialProperties defined (e.g., G4_Air), opticalphotons won't be able to
	enter the volume and they will be killed! However, G4StepStatus::fGeomBoundary can be used to extract the
	information (that's done in SteppingAction::UserSteppingAction in this project)... That's because I consider
	quite a basic geometry here. SensitiveDetectors won't be able to detect opticalphotons if there's no RINDEX defined.
	That means, for example, using "Low_density_air" instead of "G4_AIR" would solve that problem
	*/
	G4Rotate3D detRot{ m_detectorAngle, G4ThreeVector{ -1.,0.,0. } };
	G4Translate3D detTranslate{ G4ThreeVector{ 0.,0.,m_detectorDistance + detThickness } };
	G4Transform3D detTransform{ detRot * detTranslate }; //first rotate and then translate in the new coordinate system
	new G4PVPlacement{ detTransform, "detectorPhys", p_detectorLogic, p_worldPhys, false, 0, m_checkOverlap };
}

endChR