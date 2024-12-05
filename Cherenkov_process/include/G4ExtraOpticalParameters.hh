//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

/*
ABOUT THE HEADER
----------------

A class based around a std::unordered_map object to extend the information
attributed to logical volumes. This class is used by the G4CherenkovProcess
class to select what Cherenkov radiation model should be executed, and to
use the information about the full thickness of volumes (using logical
volumes solves the problem in case of parametrized volumes). Also, the flag
'm_exoticRIndex' is essential for using exotic refractive indices for
Cherenkov radiation. Please, don't manually change the flag, but use the
UI commands instead!
*/

#pragma once
#ifndef G4ExtraOpticalParameters_hh
#define G4ExtraOpticalParameters_hh

//G4 headers
#include "globals.hh"
#include "G4LogicalVolume.hh"
#include "G4OpticalParameters.hh"
#include "G4StandardChR_Model.hh"
#include "G4ThinTargetChR_Model.hh"
//std:: headers
#include <memory>
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <fstream>

//=======For hashing of G4LogicalVolume*=======
//struct G4LogicalHasher { //with this one, one should still play
//	std::size_t operator()(const G4LogicalVolume* key) {
//		return std::hash<typename>{}(something here...) //or some other hashing system
//	}
//};
//struct G4LogicalCompare { //this one is good to go
//	G4bool operator()(const G4LogicalVolume* lhv, const G4LogicalVolume* rhv) {
//		return *lhv == *rhv;
//	}
//};
//=======End of hashing of G4LogicalVolume*=======

class G4ExtraOpticalParameters_Messenger;
struct G4CherenkovMatData;

class G4ExtraOpticalParameters final {
	using dataType = G4CherenkovMatData;
public:
	static std::shared_ptr<G4ExtraOpticalParameters> GetInstance();
	~G4ExtraOpticalParameters();

	//=======Inlines around m_ChRMatData=======
	inline G4bool AddNewChRMatData(const G4LogicalVolume*, const dataType);
	inline G4bool AddNewChRMatData(const std::pair<const G4LogicalVolume*, dataType>);
	inline G4bool AddNewChRMatData(const G4LogicalVolume*);
	[[nodiscard]] inline dataType& FindOrCreateChRMatData(const G4LogicalVolume*);
	[[nodiscard]] inline const dataType* FindChRMatData(const G4LogicalVolume*) const;
	[[nodiscard]] inline const std::unordered_map<const G4LogicalVolume*, dataType>& GetChRMatData() const;
	//using the previous is user's responsibility, i.e., if rebuilding geometry, one should clear the map (remove const)
	void ScanAndAddUnregisteredLV();
	void PrintChRMatData(const G4LogicalVolume* aLV = nullptr) const;
private:
	G4ExtraOpticalParameters();
	G4ExtraOpticalParameters_Messenger* p_extraOpticalParameters_Messenger = nullptr;
	std::unordered_map<const G4LogicalVolume*, dataType/*, G4LogicalHasher, G4LogicalCompare*/> m_ChRMatData;
};

// To change m_exoticRIndex use the UI command; m_executeModel can be changed manually as well
struct G4CherenkovMatData {
	friend G4ExtraOpticalParameters;
	friend G4ThinTargetChR_Model;
	friend G4ExtraOpticalParameters_Messenger;
public:
	explicit G4CherenkovMatData(const size_t execModel = 0)
		: m_executeModel(execModel), m_halfThickness(-1000.), m_exoticRIndex(false),
		m_exoticFlagInital(false), m_minAxis(255) {}
	~G4CherenkovMatData() { delete p_middlePoint; }

	// G4CherenkovProcess model ID that will be executed
	size_t m_executeModel;

public:
	// return copies
	[[nodiscard]] inline G4ThreeVector GetMiddlePointVec() const;
	[[nodiscard]] inline G4double GetHalfThickness() const;
	[[nodiscard]] inline G4bool GetExoticRIndex() const;
	[[nodiscard]] inline G4bool GetExoticInitialFlag() const;
	[[nodiscard]] inline unsigned char GetMinAxis() const;
private:
	//only friends may access
	G4ThreeVector* p_middlePoint = nullptr;
	// The previous is needed when transforming into the coordinate system of a layered radiator
	// Also, using a pointer as I believe it could save some memory (but again that depends on the use case)
	G4double m_halfThickness;
	G4bool m_exoticRIndex;
	G4bool m_exoticFlagInital;
	unsigned char m_minAxis; // 0 - x min; 1 - y min; 2 - z min; >2 fail
	// most of the private members are needed only for G4ThinTargetChR_Model
	// 5 wasted bytes on x64
};

//=======Inlines around m_ChRMatData=======

G4bool G4ExtraOpticalParameters::AddNewChRMatData(const G4LogicalVolume* key, const dataType val) {
	return m_ChRMatData.insert(std::make_pair(key, val)).second;
}

G4bool G4ExtraOpticalParameters::AddNewChRMatData(const std::pair<const G4LogicalVolume*, dataType> thePair) {
	return m_ChRMatData.insert(thePair).second;
}

G4bool G4ExtraOpticalParameters::AddNewChRMatData(const G4LogicalVolume* key) {
	return m_ChRMatData.insert(std::make_pair(key, dataType{})).second;
}

G4CherenkovMatData& G4ExtraOpticalParameters::FindOrCreateChRMatData(const G4LogicalVolume* key) {
	return m_ChRMatData[key];
}

const G4CherenkovMatData* G4ExtraOpticalParameters::FindChRMatData(const G4LogicalVolume* key) const {
	try {
		return &(m_ChRMatData.at(key));
	}
	catch (std::out_of_range) {
		const char* location = "G4ExtraOpticalParameters::FindChRMatData(const G4LogicalVolume* key)";
		std::ostringstream reason;
		reason << "Logical volume " << std::quoted(key->GetName()) << " was not loaded into m_ChRMatData\n";
		G4Exception(location, "WE_ExtraOParam01", JustWarning, reason);
		return nullptr;
	}
	catch (...) {
		G4Exception("G4ExtraOpticalParameters::FindChRMatData", "FE_ExtraOParam01", FatalException, "This one was not expected!\n");
		return nullptr; //silence compiler
	}
}

const std::unordered_map<const G4LogicalVolume*, G4CherenkovMatData>& G4ExtraOpticalParameters::GetChRMatData() const {
	return m_ChRMatData;
}

//=======Get inlines for G4CherenkovMatData=======
G4ThreeVector G4CherenkovMatData::GetMiddlePointVec() const {
	if (p_middlePoint)
		return G4ThreeVector{ *p_middlePoint };
	else
		return G4ThreeVector{ DBL_MAX, DBL_MAX, DBL_MAX };
}
G4double G4CherenkovMatData::GetHalfThickness() const {
	return m_halfThickness;
}
G4bool G4CherenkovMatData::GetExoticRIndex() const {
	return m_exoticRIndex;
}
G4bool G4CherenkovMatData::GetExoticInitialFlag() const {
	return m_exoticFlagInital;
}
unsigned char G4CherenkovMatData::GetMinAxis() const {
	return m_minAxis;
}

#endif // !G4ExtraOpticalParameters_hh