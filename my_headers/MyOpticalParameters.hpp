#pragma once
#ifndef MyOpticalParameters_hpp
#define MyOpticalParameters_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "DefsNConsts.hpp"
#include "UnitsAndBench.hpp"
#include "AccessPhysicsVectors.hpp"
#include "MyOpticalParameters_Messenger.hpp"
//G4 headers
#include "G4OpticalParameters.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4Material.hh"
//std:: headers
#include <cstddef>
#include <vector>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_set>

beginChR

//struct G4LogicalHasher { //with this one, one should still play
//	std::size_t operator()(const G4LogicalVolume* key) {
//		return std::hash<typename>{}(something here...) //or some other hashing system
//	}
//};
//struct G4LogicalCompare { //this one is good to go
//	bool operator()(const G4LogicalVolume* lhv, const G4LogicalVolume* rhv) {
//		return *lhv == *rhv;
//	}
//};

//I'm considering only mutli-threaded version of Geant4 here
//G4OpticalParameters doesn't have any virtuals so don't inherit!
enum class ChRModelIndex {
	Default, //the process class will figure out what model to use
	AlmostOriginalChR, //force AlmostOriginalChRModel
	TammThinTargetChR, //force TammThinTarget_Model
	thePCMChR //force thePCM_Model - DO NOT USE THIS ONE, IT HAS NOT BEEN CREATED YET!
};

inline std::ostream& operator<<(std::ostream& outS, const ChRModelIndex modelIndex) {
	switch (modelIndex)
	{
	case ChR::ChRModelIndex::Default:
		outS << "Default";
		break;
	case ChR::ChRModelIndex::AlmostOriginalChR:
		outS << "AlmostOriginalChR";
		break;
	case ChR::ChRModelIndex::TammThinTargetChR:
		outS << "TammThinTargetChR";
		break;
	case ChR::ChRModelIndex::thePCMChR:
		outS << "thePCMChR";
		break;
	default:
		outS << "How??";
		break;
	}
	return outS;
}

template<typename T>
struct CherenkovMatData {
	static_assert(std::is_enum_v<T> == true, "The class MyOpticalParameters<T> is designed to support enum tpye of T!\n");
	//for now keeping it simple, but this struct might be changed in the future
	explicit CherenkovMatData(T toForce = T::Default, const double val = -1)
	: m_forceModel(toForce), m_matThickness(val), m_maxRIndex(-1.) {}
	~CherenkovMatData() = default;
	//m_matThickness should be manually set!!!
	double m_matThickness; //for now keeping it only in a single dimension so one should be careful. thePCM might allow to consider all three dimensions.
	double m_maxRIndex;
	T m_forceModel;
	//VerboseLevelMessenger<T>* p_verbose = nullptr;
};

template <typename T>
class MyOpticalParameters_Messenger;

template<typename T>
class MyOpticalParameters final {
public:
	using dataType = CherenkovMatData<T>;
	static_assert(std::is_enum_v<T> == true, "The class MyOpticalParameters is designed to support enum tpye of T!");
	static std::shared_ptr<MyOpticalParameters> GetInstance();
	~MyOpticalParameters();
	//=======Methods around m_ChRMatData=======
	inline bool AddNewChRMatData(const G4LogicalVolume*, const dataType);
	inline bool AddNewChRMatData(const std::pair<const G4LogicalVolume*, dataType>);
	inline bool AddNewChRMatData(const G4LogicalVolume*);
	_NODISCARD inline dataType& FindOrCreateChRMatData(const G4LogicalVolume*);
	_NODISCARD inline dataType* FindChRMatData(const G4LogicalVolume*);
	_NODISCARD inline const std::unordered_map<const G4LogicalVolume*, dataType>& GetChRMatData() const;
	//using the previous is user's responsibility, i.e., if rebuilding geometry, one should clear the map (remove const)
	void ScanAndAddUnregisteredLV();
	//=======Set inlines=======
	inline int SetVerboseLevel(const int value);
	inline void SetForcedThinTargetCondition(const bool);
	inline double SetThinTargetLimit(const double);
	//=======Get inlines=======
	_NODISCARD inline int GetVerboseLevel() const;
	_NODISCARD inline bool GetForcedThinTargetCondition() const;
	_NODISCARD inline double GetThinTargetLimit() const;
private:
	MyOpticalParameters();
	//no need for hashing (std::hash<G4LogicalVolume>) -> pointers are std hashable
	//still, an example of hasing is provided under comments for those willing to play
	std::unordered_map<const G4LogicalVolume*, dataType/*, G4LogicalHasher, G4LogicalCompare*/> m_ChRMatData;
	//For now leaving G4LogicalVolume* as a key
	//In the future might change it to std::unordered_map<typename TT*, CherenkovMatData<T>> to include G4VPhysicalVolume or something
	//else as well. There might be much greater number of G4VPhysicalVolume instances, but that's why unordered_map is used
	bool m_stronglyForcedThinTarget;
	double m_thinTargetLimit;
	//A standard ChR model will be used for materials with CherenkovMatData<T>::
	//m_maxThickness > m_thinTargetLimit and ChRModelIndex::Default, at least
	//unless m_stronglyForcedThinTarget == true
	int m_veroseLevel;
	MyOpticalParameters_Messenger<T>* p_myOptParamMessenger = nullptr;
};

//=======Methods around m_ChRMatData=======
template<typename T>
bool MyOpticalParameters<T>::AddNewChRMatData(const G4LogicalVolume* key, const dataType val) {
	return m_ChRMatData.insert(std::make_pair(key, val)).second;
}
template<typename T>
bool MyOpticalParameters<T>::AddNewChRMatData(const std::pair<const G4LogicalVolume*, dataType> thePair) {
	return m_ChRMatData.insert(thePair).second;
}
template<typename T>
bool MyOpticalParameters<T>::AddNewChRMatData(const G4LogicalVolume* key) {
	return m_ChRMatData.insert(std::make_pair(key, dataType{})).second;
}
template<typename T>
CherenkovMatData<T>& MyOpticalParameters<T>::FindOrCreateChRMatData(const G4LogicalVolume* key) {
	return m_ChRMatData[key];
}
template<typename T>
CherenkovMatData<T>* MyOpticalParameters<T>::FindChRMatData(const G4LogicalVolume* key) {
	try {
		return &(m_ChRMatData.at(key));
	}
	catch (std::out_of_range) {
		const char* location = "ChR::MyOpticalParameters<T>::FindChRMatData(const G4LogicalVolume* key)";
		std::ostringstream reason;
		reason << "Logical volume " << std::quoted(key->GetName()) << " was not loaded into m_ChRMatData\n";
		G4Exception(location, "WE1009", JustWarning, reason);
		return nullptr;
	}
	catch (...) {
		G4Exception("ChR::MyOpticalParameters<T>::FindChRMatData(...)", "FE1012", FatalException, "This one was not expected!\n");
		return nullptr; //silence compiler
	}
}
template<typename T>
const std::unordered_map<const G4LogicalVolume*, CherenkovMatData<T>>& MyOpticalParameters<T>::GetChRMatData() const {
	return m_ChRMatData;
}
//=======Set inlines=======
template<typename T>
void MyOpticalParameters<T>::SetForcedThinTargetCondition(const bool value) {
	m_stronglyForcedThinTarget = value;
}
template<typename T>
int MyOpticalParameters<T>::SetVerboseLevel(const int value) {
	int temp = m_veroseLevel;
	m_veroseLevel = value;
	G4OpticalParameters::Instance()->SetVerboseLevel(value);
	return temp;
}
template<typename T>
double MyOpticalParameters<T>::SetThinTargetLimit(const double value) {
	double temp = m_thinTargetLimit;
	m_thinTargetLimit = value;
	return temp;
}
//=======Get inlines=======
template<typename T>
int MyOpticalParameters<T>::GetVerboseLevel() const {
	return m_veroseLevel;
}
template<typename T>
bool MyOpticalParameters<T>::GetForcedThinTargetCondition() const {
	return m_stronglyForcedThinTarget;
}
template<typename T>
double MyOpticalParameters<T>::GetThinTargetLimit() const {
	return m_thinTargetLimit;
}

//=========public ChR::MyOpticalParameters:: methods=========
template <typename T>
std::shared_ptr<MyOpticalParameters<T>> MyOpticalParameters<T>::GetInstance() {
	static std::shared_ptr<MyOpticalParameters<T>> instance {new MyOpticalParameters<T>{}};
	return instance;
}

template <typename T>
MyOpticalParameters<T>::~MyOpticalParameters() {
	delete p_myOptParamMessenger;
}

template<typename T>
void MyOpticalParameters<T>::ScanAndAddUnregisteredLV() {
	G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
	for (size_t i = 0; i < lvStore->size(); i++) {
		G4LogicalVolume* logVolume = (*lvStore)[i];
		CherenkovMatData<T>& matData = m_ChRMatData[logVolume];
		if (matData.m_maxRIndex == -1.) {
			const G4MaterialPropertiesTable* matPropTab = logVolume->GetMaterial()->GetMaterialPropertiesTable();
			if (matPropTab) {
				G4PhysicsFreeVector* physVec = matPropTab->GetProperty(kRINDEX);
				if (!physVec)
					physVec = matPropTab->GetProperty(kREALRINDEX);
				if(physVec)
					matData.m_maxRIndex = reinterpret_cast<AccessPhysicsVector*>(physVec)->GetRealDataVectorMax();
			}
		}
	}
}

//=========private ChR::MyOpticalParameters:: methods=========
template<typename T>
MyOpticalParameters<T>::MyOpticalParameters()
: m_stronglyForcedThinTarget(false),
m_veroseLevel(0), m_thinTargetLimit(1000._um){
	p_myOptParamMessenger = new MyOpticalParameters_Messenger<T>{ this };
	G4OpticalParameters::Instance(); //can't just copy for other than new Cherenkov processes
}

endChR

#endif // !MyOpticalParameters_hpp