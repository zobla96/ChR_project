#pragma once
#ifndef DetectorConstruction_hpp
#define DetectorConstruction_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "MyOpticalParameters.hpp"
#include "DetectorConstruction_Messenger.hpp"
//G4 headers
#include "G4VUserDetectorConstruction.hh"
//...
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SDManager.hh"
#include "G4VisAttributes.hh"
#include "G4Isotope.hh"
#include "G4Region.hh"
//std:: headers
#include <algorithm>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

beginChR

class DetectorConstruction_Messenger;

class DetectorConstruction final : public G4VUserDetectorConstruction {
public:
	DetectorConstruction();
	~DetectorConstruction() override;
	static DetectorConstruction* GetInstance();
	G4VPhysicalVolume* Construct() override;
	//=======Set inlines=======
	inline double SetRadiatorAngle(const double value);
	inline double SetRadiatorThickness(const double value);
	inline unsigned char SetNoOfRadLayers(const unsigned char value);
	inline void SetRadiatorMaterialName(const G4String&);
	inline double SetDetectorRadius(const double value);
	inline double SetDetectorAngle(const double value);
	inline double SetDetectorDistance(const double value);
	inline bool SetCheckOverlap(const bool value);
	inline unsigned char SetVerboseLevel(const unsigned char value);
	//=======Get inlines=======
	_NODISCARD inline double GetRadiatorAngle() const;
	_NODISCARD inline double GetRadiatorThickness() const;
	_NODISCARD inline unsigned char GetNoOfRadLayers() const;
	_NODISCARD inline const G4String& GetRadiatorMaterialName() const;
	_NODISCARD inline const unsigned char& GetRefNoOfRadLayers() const;
	_NODISCARD inline double GetDetectorRadius() const;
	_NODISCARD inline double GetDetectorAngle() const;
	_NODISCARD inline double GetDetectorDistance() const;
	_NODISCARD inline bool GetCheckOverlap() const;
	_NODISCARD inline unsigned char GetVerboseLevel() const;
private:
	//Dummy functions
	void LoadMaterials();
	void LoadWorld();
	void LoadRadiator();
	void LoadDetector() const;
	//private variables
	double m_radiatorAngle;
	double m_radiatorThickness; //it's a half-thickness
	G4String m_radiatorMaterial;
	double m_detectorRadius;
	double m_detectorAngle;
	double m_detectorDistance;
	bool m_checkOverlap;
	unsigned char m_noOfRadLayers; //even 255 layers is too many for our needs
	unsigned char m_verbose; //there's no G4VUserDetectorConstruction::verbose member
	//Additional pointers
	G4VPhysicalVolume* p_worldPhys = nullptr;
	std::unique_ptr<G4VisAttributes> m_visAttrHide;
	DetectorConstruction_Messenger* p_detConstrMessenger = nullptr;
};

//=======Set inlines=======
double DetectorConstruction::SetRadiatorAngle(const double value) {
	double temp = m_radiatorAngle;
	m_radiatorAngle = value;
	return temp;
}
double DetectorConstruction::SetRadiatorThickness(const double value) {
	double temp = m_radiatorThickness;
	m_radiatorThickness = value;
	return temp;
}
unsigned char DetectorConstruction::SetNoOfRadLayers(const unsigned char value) {
	unsigned char temp = m_noOfRadLayers;
	m_noOfRadLayers = value;
	return temp;
}
void DetectorConstruction::SetRadiatorMaterialName(const G4String& value) {
	m_radiatorMaterial = value;
}
double DetectorConstruction::SetDetectorRadius(const double value) {
	double temp = m_detectorRadius;
	m_detectorRadius = value;
	return temp;
}
double DetectorConstruction::SetDetectorAngle(const double value) {
	double temp = m_detectorAngle;
	m_detectorAngle = value;
	return temp;
}
double DetectorConstruction::SetDetectorDistance(const double value) {
	double temp = m_detectorDistance;
	m_detectorDistance = value;
	return temp;
}
bool DetectorConstruction::SetCheckOverlap(const bool value) {
	bool temp = m_checkOverlap;
	m_checkOverlap = value;
	return temp;
}
unsigned char DetectorConstruction::SetVerboseLevel(const unsigned char value) {
	unsigned char temp = m_verbose;
	m_verbose = value;
	return temp;
}

//=======Get inlines=======
double DetectorConstruction::GetRadiatorAngle() const {
	return m_radiatorAngle;
}
double DetectorConstruction::GetRadiatorThickness() const {
	return m_radiatorThickness;
}
unsigned char DetectorConstruction::GetNoOfRadLayers() const {
	return m_noOfRadLayers;
}
const G4String& DetectorConstruction::GetRadiatorMaterialName() const {
	return m_radiatorMaterial;
}
const unsigned char& DetectorConstruction::GetRefNoOfRadLayers() const {
	return m_noOfRadLayers;
}
double DetectorConstruction::GetDetectorRadius() const {
	return m_detectorRadius;
}
double DetectorConstruction::GetDetectorAngle() const {
	return m_detectorAngle;
}
double DetectorConstruction::GetDetectorDistance() const {
	return m_detectorDistance;
}
bool DetectorConstruction::GetCheckOverlap() const {
	return m_checkOverlap;
}
unsigned char DetectorConstruction::GetVerboseLevel() const {
	return m_verbose;
}

enum class EnergyValue {
	Wavelength,
	Energy
};

//========Helper struct=======
struct HelperToBuildMatPropTable {
	template <typename... Args>
	auto FillMatPropertiesVectors(const char* fileName, G4Material* theMat, const EnergyValue enValue, const double enUnit, const char separator, Args... args)
		-> std::enable_if_t<(std::is_same_v<const char*, Args> && ...)>;
	//Args can be anything, F is lambda, function pointer or anything of that kind... not sure about type_traits to check the F during compile-time
	//On the other hand, the code cannot run if it's not a function
	template<typename F, typename... Args>
	void FillMatPropData(G4Material* theMat, const char* matProperty, F&& f, double minEnergy, double maxEnergy, size_t energyPoints, Args&&... args);
private:
	//this one is privite, no need for safeties
	template<typename T, typename... Args>
	void LookForError(std::unordered_map<const char*, std::vector<double>>& theMap, T val, Args... args);
};

template <typename T, typename... Args>
void HelperToBuildMatPropTable::LookForError(std::unordered_map<const char*, std::vector<double>>& theMap, T val, Args... args) {
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
	const double enUnit, const char separator, Args... args) -> std::enable_if_t<(std::is_same_v<const char*, Args> && ...)> {
	//make sure to use "Energy" argument if you are filling matProps this way
	std::ostringstream theError;
	if constexpr (sizeof...(args) < 2) { //warning error - the code may continue without mat properties
		theError << "Too few arguments...\nThe code is proceeding without any material properties loaded\n";
		G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors()", "WE1004", JustWarning, theError);
		return;
	}
	std::string matName;
	matName = "Material: " + theMat->GetName();
	std::ifstream iFS;
	iFS.open(fileName, std::ios::in);
	if (!iFS) { //fatal error if not opened
		theError << "The file with material properties data could not be opened\n";
		G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors()", "FE1005", FatalException, theError);
	}
	bool found_Energy_arg = false;
	size_t counter = 0;
	//unordered_map might not be the best choice here because most users will use just a few parameters (e.g. only RIndex);
	//therefore, it might be slightly slower than simply vectors...
	//still this works as well and won't affect G4 runs so no need to optimize further
	std::unordered_map<const char*, std::vector<double>> theData;
	std::vector<const char*> helperVec; //More complex syntax with Args... args
	(theData.insert(std::make_pair(args, std::vector<double> {})), ...);
	(helperVec.emplace_back(args), ...);
	for (auto& [key, value] : theData) {
		if (key == "Energy")
			found_Energy_arg = true;
		value.reserve(100);
	}
	if (!found_Energy_arg) {
		theError << "You must use \"Energy\" argument to load material properties through this function\n"
			<< "The code is proceeding without any material properties loaded\n";
		G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors()", "WE1005", JustWarning, theError);
		return;
	}
	if constexpr (sizeof...(args) != 0) {
		if (sizeof...(args) != theData.size()) { //fatal error
			LookForError(theData, args...); //using helperVector to evade this - it's much uglier I think
			theError << "A multiple use of a unique material property has been detected. You used:\n";
			for (const auto& [key, value] : theData)
				theError << std::quoted(key)<< "\t-->\t" << value.size() << "x times\n";
			G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors()", "FE1006", FatalException, theError);
		}
	}
	std::string line, value;
	std::istringstream iSS;
	while (true) {
		if (iFS.eof()) {
			theError << "The material hasn't been found\nThe code is proceeding without any material properties loaded\n";
			G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors()", "WE1006", JustWarning, theError);
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
				theError	<< "Too few data to use them for material properties vectors...\n"
							<< "The code is proceeding without any material properties loaded\n";
				G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors()", "WE1007", JustWarning, theError);
				return;
			}
			break;
		}
		for (const char* i : helperVec) {
			if (iSS.peek() == EOF) {
				theError	<< "The data is missing - each line should have " << sizeof...(args) << " double values\n"
							<< "Successfully loaded data are:\n";
				PrintLoadedData();
				theError << "\nThe line:\n\"" << line << "\"\nis missing data\n";
				G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors()", "FE1007", FatalException, theError);
			}
			std::getline(iSS, value, separator);
			try {
				if (i == "Energy") {
					double temp;
					if (enValue == EnergyValue::Wavelength) {
						temp = 1.239841984e-6 * m * eV / (std::stod(value) * enUnit); //h * c = 1.239841984e-6 * m * eV
						theData[i].emplace_back(temp);
					}
					//else can be used but I got no idea if there will be more EnergyValue parameters in the future
					else /*if (enValue == EnergyValue::Energy)*/
						theData[i].emplace_back(std::stod(value) * enUnit);
				}
				else //might be better to go with std::pair to include units for all data, but I don't know if they are needed or exist
					theData[i].emplace_back(std::stod(value));
			}
			catch (std::out_of_range) {
				theError << "The data is out of range for double values.\n"
					<< "Double values range from " << DBL_MIN << " to " << DBL_MAX << '\n'
					<< "Successfully loaded data are:\n";
				PrintLoadedData();
				theError << "\nThe line:\n\"" << line << "\"\ncannot be converted\n";
				G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors()", "FE1008", FatalException, theError);
			}
			catch (std::invalid_argument) {
				theError << "The data cannot be converted double type.\n"
					<< "Successfully loaded data are:\n";
				PrintLoadedData();
				theError << "\nThe line:\n\"" << line << "\"\ncannot be converted\n";
				G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors()", "FE1009", FatalException, theError);
			}
			catch (...) {
				theError << "This one should not have happen\n";
				G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors()", "FE1010", FatalException, theError);;
			}
		}
		if (iSS.peek() != EOF) {
			theError << "An exess of the data - each line should have " << sizeof...(args) << " double values\n"
				<< "Successfully loaded data are:\n";
			PrintLoadedData();
			theError << "\t\nThe line:\n\"" << line << "\"\nhas a data excess\n\n";
			G4Exception("ChR::DetectorConstruction::FillMatPropertiesVectors()", "FE1011", FatalException, theError);
		}
		counter++;
	}
	G4MaterialPropertiesTable* theProperties = theMat->GetMaterialPropertiesTable();
	if (!theProperties) {
		theProperties = new G4MaterialPropertiesTable{};
		for (const auto& [key, value] : theData) {
			if (key == "Energy")
				continue;
			theProperties->AddProperty(key, theData["Energy"], value);
		}
		theMat->SetMaterialPropertiesTable(theProperties);
	}
	else {
		for (const auto& [key, value] : theData) {
			if (key == "Energy")
				continue;
			theProperties->AddProperty(key, theData["Energy"], value);
		}
	}

	//print loaded data - I think there's no need for verbose here
	std::cout << "The properties loaded for: " << theMat->GetName() << '\n';
	std::cout << '{';
	for (const auto& i : helperVec) {
		std::cout << i;
		if (*(helperVec.end() - 1) == i)
			break;
		std::cout << ", ";
	}
	std::cout << "}\n";
	for (size_t i = 0; i < counter; i++) {
		for (const auto& j : helperVec)
			std::cout << theData[j][i] << '\t';
		std::cout << '\n';
	}
	std::cout << std::endl;
}

template<typename F, typename... Args>
void HelperToBuildMatPropTable::FillMatPropData(G4Material* theMat, const char* matProperty ,F&& f, double minEnergy, double maxEnergy, size_t energyPoints, Args&&... args) {
	//used to pass a function f(energy, args...)
	std::vector<double> theEnergyData;
	std::vector<double> theData;
	double enrgyStep = (maxEnergy - minEnergy) / (energyPoints - 1);
	if (enrgyStep <= 0) {
		std::ostringstream err;
		err << "Cannot load MaterialPropertiesData for " << theMat->GetName() << " because energy step has wrong values\n"
			<< "Pay attention to argument order of the given function\nProceeding without any MaterialProperties loaded...\n";
		G4Exception("HelperToBuildMatPropTable::FillMatPropData(...)", "WE1008", JustWarning, err);
	}
	for (size_t i = 0; i < energyPoints; i++) {
		double energy = minEnergy + i * enrgyStep;
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
	
	//print loaded data - I think there's no need for verbose here
	std::cout << "The properties {Energy, " << matProperty << "} loaded for: " << theMat->GetName() << '\n';
	for (size_t i = 0; i < energyPoints; i++)
		std::cout << theEnergyData[i] << '\t' << theData[i] << '\n';
	std::cout << std::endl;
}
endChR

#endif // !DetectorConstruction_hpp