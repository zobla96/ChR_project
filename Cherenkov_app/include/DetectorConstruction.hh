//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef DetectorConstruction_hh
#define DetectorConstruction_hh

// user headers
#include "DetectorConstruction_Messenger.hh"
#include "DefsNConsts.hh"
#include "PhysicsList.hh"
// G4 headers
#include "G4VUserDetectorConstruction.hh"
// ...
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4Material.hh"
// std:: headers
#include <algorithm>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string_view>
#include <memory>

class G4VPhysicalVolume;

beginChR

class DetectorConstruction_Messenger;

class DetectorConstruction final : public G4VUserDetectorConstruction {
public:
	DetectorConstruction();
	~DetectorConstruction() override;

	DetectorConstruction(const DetectorConstruction&) = delete;
	DetectorConstruction& operator=(const DetectorConstruction&) = delete;
	DetectorConstruction(DetectorConstruction&&) = delete;
	DetectorConstruction& operator=(DetectorConstruction&&) = delete;

	G4VPhysicalVolume* Construct() override;
	//=======Set inlines=======
	inline void SetRadiatorAngle(const G4double value);
	inline void SetRadiatorThickness(const G4double value);
	inline void SetRadiatorMaterialName(const G4String&);
	inline void SetDetectorRadius(const G4double value);
	inline void SetDetectorAngle(const G4double value);
	inline void SetDetectorDistance(const G4double value);
	//=======Get inlines=======
	[[nodiscard]] inline G4double GetRadiatorAngle() const;
	[[nodiscard]] inline G4double GetRadiatorThickness() const;
	[[nodiscard]] inline const G4String& GetRadiatorMaterialName() const;
	[[nodiscard]] inline G4double GetDetectorRadius() const;
	[[nodiscard]] inline G4double GetDetectorAngle() const;
	[[nodiscard]] inline G4double GetDetectorDistance() const;
private:
	//Dummy functions
	void LoadMaterials();
	void LoadWorld();
	void LoadRadiator();
	void LoadDetector() const;
	//Additional pointers
	G4VPhysicalVolume* p_worldPhys = nullptr;
	std::unique_ptr<G4VisAttributes> m_visAttrHide;
	DetectorConstruction_Messenger* p_detConstructionMessenger = nullptr;
	//private variables
	G4String m_radiatorMaterialName;
	G4double m_radiatorAngle;
	G4double m_radiatorThickness; // half-thickness
	G4double m_detectorRadius;
	G4double m_detectorAngle;
	G4double m_detectorDistance;
};

//=======Set inlines=======
void DetectorConstruction::SetRadiatorAngle(const G4double value) {
	m_radiatorAngle = value;
}
void DetectorConstruction::SetRadiatorThickness(const G4double value) {
	m_radiatorThickness = value;
}
void DetectorConstruction::SetRadiatorMaterialName(const G4String& value) {
	m_radiatorMaterialName = value;
}
void DetectorConstruction::SetDetectorRadius(const G4double value) {
	m_detectorRadius = value;
}
void DetectorConstruction::SetDetectorAngle(const G4double value) {
	m_detectorAngle = value;
}
void DetectorConstruction::SetDetectorDistance(const G4double value) {
	m_detectorDistance = value;
}

//=======Get inlines=======
G4double DetectorConstruction::GetRadiatorAngle() const {
	return m_radiatorAngle;
}
G4double DetectorConstruction::GetRadiatorThickness() const {
	return m_radiatorThickness;
}
const G4String& DetectorConstruction::GetRadiatorMaterialName() const {
	return m_radiatorMaterialName;
}
G4double DetectorConstruction::GetDetectorRadius() const {
	return m_detectorRadius;
}
G4double DetectorConstruction::GetDetectorAngle() const {
	return m_detectorAngle;
}
G4double DetectorConstruction::GetDetectorDistance() const {
	return m_detectorDistance;
}

enum class EnergyValue {
	Wavelength,
	Energy
};

//========Helper struct=======
struct HelperToBuildMatPropTable {
	template <typename... Args>
	auto FillMatPropertiesVectors(const char* fileName, G4Material* theMat, const EnergyValue enValue, const G4double enUnit, const char separator, Args... args)
		-> std::enable_if_t<(std::is_same_v<const char*, Args> && ...)>;
	// Args can be anything, F is lambda, function pointer or anything of that kind... not sure about type_traits to check the F during compile-time...
	// On the other hand, the code cannot compile if it's not a function
	template<typename F, typename... Args>
	void FillMatPropData(G4Material* theMat, const char* matProperty, F&& f, G4double minEnergy, G4double maxEnergy, size_t energyPoints, Args&&... args);
private:
	template<typename T, typename... Args>
	void LookForError(std::unordered_map<std::string_view, std::vector<G4double>>& theMap, T val, Args... args);
};

template <typename T, typename... Args>
void HelperToBuildMatPropTable::LookForError(std::unordered_map<std::string_view, std::vector<G4double>>& theMap, T val, Args... args) {
	if constexpr (sizeof...(args) == 0) {
		if (auto i = theMap.find(val); i != theMap.end())
			i->second.emplace_back(0.);
	}
	else {
		if (auto i = theMap.find(val); i != theMap.end())
			i->second.emplace_back(0.);
		LookForError(theMap, args...);
	}
}

template<typename... Args>
auto HelperToBuildMatPropTable::FillMatPropertiesVectors(const char* fileName, G4Material* theMat, const EnergyValue enValue,
	const G4double enUnit, const char separator, Args... args) -> std::enable_if_t<(std::is_same_v<const char*, Args> && ...)> {
	//make sure to use "Energy" argument if you are filling matProps this way
	std::ostringstream theError;
	if constexpr (sizeof...(args) < 2) { //warning error - the code may continue without mat properties
		theError << "Too few arguments...\nThe code is proceeding without any material properties loaded\n";
		G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors", "WE_HelpMatPTable01", JustWarning, theError);
		return;
	}
	std::string matName;
	matName = "Material: " + theMat->GetName();
	std::ifstream iFS;
	iFS.open(fileName, std::ios::in);
	if (!iFS) { //fatal error if not opened
		theError << "The file with material properties data could not be opened\n";
		G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors", "FE_HelpMatPTable01", FatalException, theError);
	}
	G4bool found_Energy_arg = false;
	size_t counter = 0;
	// unordered_map might not be the best choice here because mostly one will use just a few parameters (e.g. only RIndex);
	// therefore, it might be slightly slower than simply vectors...
	// still this works as well and won't affect G4 runs as it's pre-run time
	// WARNING: it's ok to use const char* with /GF flag enabled (standard for /O1 /O2)! That is:
	// std::unordered_map<const char*, std::vector<G4double>> theData;
	std::unordered_map<std::string_view, std::vector<G4double>> theData;
	std::vector<const char*> helperVec; // More complex syntax with Args... args
	(theData.insert(std::make_pair(std::string_view{ args }, std::vector<G4double> {})), ...);
	(helperVec.emplace_back(args), ...);
	for (auto& [key, value] : theData) {
		if (key == "Energy")
			found_Energy_arg = true;
		value.reserve(100);
	}
	if (!found_Energy_arg) {
		theError << "You must use \"Energy\" argument to load material properties through this function\n"
			<< "The code is proceeding without any material properties loaded\n";
		G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors", "WE_HelpMatPTable02", JustWarning, theError);
		return;
	}
	if constexpr (sizeof...(args) != 0) {
		if (sizeof...(args) != theData.size()) { // fatal error
			LookForError(theData, args...); // using helperVec to evade this - it's much uglier I think
			theError << "A multiple use of a unique material property has been detected. You used:\n";
			for (const auto& [key, value] : theData)
				theError << std::quoted(key) << "\t-->\t" << value.size() << "x times\n";
			G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors", "FE_HelpMatPTable02", FatalException, theError);
		}
	}
	G4String line, value;
	std::istringstream iSS;
	while (true) {
		if (iFS.eof()) {
			theError << "The material" << std::quoted(theMat->GetName()) << "hasn't been found in the input file\n"
				"The code is proceeding without any material properties loaded\n";
			G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors", "WE_HelpMatPTable03", JustWarning, theError);
			return;
		}
		std::getline(iFS, line, '\n');
		if (line.find(matName) != line.npos)
			break;
	}
	std::function<void()> PrintLoadedData = [&] {
		for (size_t i = 0; i < counter; i++) {
			for (const char* j : helperVec)
				theError << theData[j][i] << '\t';
			theError << '\n';
		}
	};
	while (true) {
		std::getline(iFS, line, '\n');
		iSS.str(line);
		iSS.clear();
		iSS.seekg(0);
		if (line.empty() || line.find("Data end") != line.npos || iFS.eof()) {
			iFS.close();
			if (theData[helperVec[0]].size() < 2) {
				theError << "Too few data to use them for material properties vectors...\n"
					<< "The code is proceeding without any material properties loaded\n";
				G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors", "WE_HelpMatPTable04", JustWarning, theError);
				return;
			}
			break;
		}
		for (const char* i : helperVec) {
			if (iSS.peek() == EOF) {
				theError << "The data is missing - each line should have " << sizeof...(args) << " double values\n"
					<< "Successfully loaded data are:\n";
				PrintLoadedData();
				theError << "\nThe line:\n\"" << line << "\"\nis missing data\n";
				G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors", "FE_HelpMatPTable03", FatalException, theError);
			}
			std::getline(iSS, value, separator);
			try {
				if (std::strcmp(i, "Energy") == 0) {
					G4double temp;
					if (enValue == EnergyValue::Wavelength) {
						temp = 1.239841984e-6 * m * eV / (std::stod(value) * enUnit); // h * c = 1.239841984e-6 * m * eV
						theData[i].emplace_back(temp);
					}
					else /*if (enValue == EnergyValue::Energy)*/
						theData[i].emplace_back(std::stod(value) * enUnit);
				}
				else
					theData[i].emplace_back(std::stod(value));
			}
			catch (std::out_of_range) {
				theError << "The data is out of range for double values.\n"
					<< "Double values range from " << -DBL_MAX << " to " << DBL_MAX << '\n'
					<< "Successfully loaded data are:\n";
				PrintLoadedData();
				theError << "\nThe line:\n\"" << line << "\"\ncannot be converted\n";
				G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors", "FE_HelpMatPTable04", FatalException, theError);
			}
			catch (std::invalid_argument) {
				theError << "The data cannot be converted to double type.\n"
					<< "Successfully loaded data are:\n";
				PrintLoadedData();
				theError << "\nThe line:\n\"" << line << "\"\ncannot be converted\n";
				G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors", "FE_HelpMatPTable05", FatalException, theError);
			}
			catch (...) {
				theError << "This one should not have happened\n";
				G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors", "FE_HelpMatPTable06", FatalException, theError);;
			}
		}
		if (iSS.peek() != EOF) {
			theError << "An excess of the data - each line should have " << sizeof...(args) << " double values\n"
				<< "Successfully loaded data are:\n";
			PrintLoadedData();
			theError << "\t\nThe line:\n\"" << line << "\"\nhas a data excess\n\n";
			G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors", "FE_HelpMatPTable07", FatalException, theError);
		}
		counter++;
	}
	G4MaterialPropertiesTable* theProperties = theMat->GetMaterialPropertiesTable();
	if (!theProperties) {
		theProperties = new G4MaterialPropertiesTable{};
		for (const auto& [key, aProperty] : theData) {
			if (key == "Energy")
				continue;
			theProperties->AddProperty(key.data(), theData["Energy"], aProperty);
		}
		theMat->SetMaterialPropertiesTable(theProperties);
	}
	else {
		for (const auto& [key, aProperty] : theData) {
			if (key == "Energy")
				continue;
			theProperties->AddProperty(key.data(), theData["Energy"], aProperty);
		}
	}

	G4cout << "The properties loaded for: " << theMat->GetName() << '\n';
	G4cout << '{';
	for (const char* i : helperVec) {
		G4cout << i;
		if (*(helperVec.end() - 1) == i)
			break;
		G4cout << ", ";
	}
	G4cout << "}\n";
	for (size_t i = 0; i < counter; i++) {
		for (const char* j : helperVec)
			G4cout << theData[j][i] << '\t';
		G4cout << '\n';
	}
	G4cout << G4endl;
}

template<typename F, typename... Args>
void HelperToBuildMatPropTable::FillMatPropData(G4Material* theMat, const char* matProperty, F&& f, G4double minEnergy, G4double maxEnergy, size_t energyNodes, Args&&... args) {
	//used to pass a function f(energy, args...)
	std::vector<G4double> theEnergyData;
	std::vector<G4double> theData;
	if (energyNodes <= 1) {
		std::ostringstream err;
		err << "Cannot load MaterialPropertiesData for " << theMat->GetName() << " because you set too few energy nodes.\n";
			"You need to set this number to at least '2'!\nProceeding without any MaterialProperties loaded...\n";
		G4Exception("HelperToBuildMatPropTable::FillMatPropData", "WE_HelpMatPTable05", JustWarning, err);
		return;
	}
	G4double energyStep = (maxEnergy - minEnergy) / (G4double)(energyNodes - 1);
	if (energyStep <= 0) {
		std::ostringstream err;
		err << "Cannot load MaterialPropertiesData for " << theMat->GetName() << " because energy step has wrong values\n"
			"Pay attention to argument order of the given function\nProceeding without any MaterialProperties loaded...\n";
		G4Exception("HelperToBuildMatPropTable::FillMatPropData", "WE_HelpMatPTable06", JustWarning, err);
		return;
	}
	for (size_t i = 0; i < energyNodes; i++) {
		G4double energy = minEnergy + (G4double)i * energyStep;
		theEnergyData.push_back(energy);
		theData.push_back(f(energy, std::forward<Args>(args)...));
	}
	G4MaterialPropertiesTable* theProperties = theMat->GetMaterialPropertiesTable();
	if (!theProperties) {
		theProperties = new G4MaterialPropertiesTable{};
		theProperties->AddProperty(matProperty, theEnergyData, theData);
		theMat->SetMaterialPropertiesTable(theProperties);
	}
	else
		theProperties->AddProperty(matProperty, theEnergyData, theData);

	G4cout << "The properties {Energy, " << matProperty << "} loaded for: " << theMat->GetName() << '\n';
	for (size_t i = 0; i < energyNodes; i++)
		G4cout << theEnergyData[i] << '\t' << theData[i] << '\n';
	G4cout << G4endl;
}

endChR

#endif // !DetectorConstruction_hh