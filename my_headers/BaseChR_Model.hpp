#pragma once
#ifndef BaseChR_Model_hpp
#define BaseChR_Model_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "MyOpticalParameters.hpp"
#include "AccessPhysicsVectors.hpp"
//G4 headers
#include "G4ParticleChange.hh"
#include "G4ForceCondition.hh"
#include "G4LossTableManager.hh"
//std:: headers
#include <algorithm>

beginChR

class BaseChR_Model {
	struct PhysicsTableData;
public:
	BaseChR_Model(const char* name, unsigned char betaSteps = 20);
	virtual ~BaseChR_Model();
	BaseChR_Model(const BaseChR_Model&) = delete;
	BaseChR_Model& operator=(const BaseChR_Model&) = delete;
	BaseChR_Model(BaseChR_Model&&) /*noexcept*/ = delete;
	BaseChR_Model& operator=(BaseChR_Model&&) /*noexcept*/ = delete;
	//=======Directly related to G4Process methods=======
	_NODISCARD virtual double PostStepModelIntLength(const G4Track& aTrack, double previousStepSize, G4ForceCondition* condition);
	virtual G4VParticleChange* PostStepModelDoIt(const G4Track&, const G4Step&) = 0;
	virtual inline void PrepareModelPhysicsTable(const G4ParticleDefinition&);
	//!!!!!!!!!!!!!!!!!
	//DO NOT OVERRIDE THE FOLLOWING METHOD WITHOUT OVERRIDING PostStepModelIntLength AND CalculateAverageNumberOfPhotons...
	//OR STILL AT LEAST LOADING THE PHYSICS BaseChR PHYSICS TABLES - the code might crash!
	//!!!!!!!!!!!!!!!!!
	virtual inline void BuildModelPhysicsTable(const G4ParticleDefinition&);
	//the following two do nothing, but might change it into a possible binary loading in the future, especially for the PCM
	_NODISCARD inline virtual bool StoreModelPhysicsTable(const G4ParticleDefinition*, const G4String&, bool) { return true; }
	virtual inline bool RetrieveModelPhysicsTable(const G4ParticleDefinition*, const G4String&, bool) { return true; }
	virtual void DumpModelInfo() const;
	virtual inline void PrepareWorkerModelPhysicsTable(const G4ParticleDefinition&);
	virtual inline void BuildWorkerModelPhysicsTable(const G4ParticleDefinition&);
	void PrintChRPhysDataVec() const;
	//=======Set inlines=======
	inline unsigned char SetNoOfBetaSteps(const unsigned char);
	inline void SetVerboseLevel(const unsigned char);
	inline void SetUseModelWithEnergyLoss(const bool);
	//=======Get inlines=======
	_NODISCARD inline const char* GetChRModelName() const;
	_NODISCARD inline bool GetFiniteThicknessCondition() const;
	_NODISCARD inline unsigned char GetNoOfBetaSteps() const;
	_NODISCARD inline unsigned char GetVerboseLevel() const;
	//The following is non-const to make it more simple - without casting
	//It is the user's responsibility not to mess up if using it
	_NODISCARD inline static std::vector<PhysicsTableData>& GetChRPhysDataVec();
	_NODISCARD inline bool GetUseModelWithEnergyLoss() const;
protected:
	struct PhysicsTableData {
		PhysicsTableData() = default;
		std::vector<double> betaVector;
		std::vector<double> dataVector;
		std::vector<G4ThreeVector> photonEVector;
		//x - delta E; y - Emin for specific beta; z - Emax for specific beta
		//Emin and Emax are used to speed up calculations in DoIt for specific cases...
		//see comments in DoIt of AlmostOriginalChR_Model
	};
	_NODISCARD virtual double CalculateAverageNumberOfPhotons(const double, const double, const G4Material*);
	G4ParticleChange* p_particleChange = nullptr;
	double m_Emin; //moving energy limits from stack to class scope to speed up DoIt methods
	double m_Emax; //focus on reducing processor cycles in DoIt loops for specific cases
	const char* m_ChRModelName;
	bool m_includeFiniteThickness; //might change to bit-field int m_includeFiniteThickness : 2; in the future (to include 3 finite dimensions)
	bool m_useModelWithEnergyLoss; //might be implemented by child classes... according to Tamm theory, there's no loss, and that's how it was implemented in the past
	const G4OpticalParameters* p_G4OptParameters;
	static std::vector<PhysicsTableData> m_ChRPhysDataVec;
	unsigned char m_noOfBetaSteps; //for filling of m_ChRPhysDataVec vector. It means noNodes = noSteps + 1
	int m_noOfPhotons;
	unsigned char m_verboseLevel; //a useless one for now, might change it in the future
private:
	BaseChR_Model() = delete;
};

//=======Set inlines=======
unsigned char BaseChR_Model::SetNoOfBetaSteps(const unsigned char value) {
	unsigned char temp = m_noOfBetaSteps;
	m_noOfBetaSteps = value;
	return temp;
}
void BaseChR_Model::SetVerboseLevel(const unsigned char value) {
	m_verboseLevel = value;
}
void BaseChR_Model::SetUseModelWithEnergyLoss(const bool val) {
	m_useModelWithEnergyLoss = val;
}

//=======Get inlines=======
const char* BaseChR_Model::GetChRModelName() const {
	return m_ChRModelName;
}
bool BaseChR_Model::GetFiniteThicknessCondition() const {
	return m_includeFiniteThickness;
}
unsigned char BaseChR_Model::GetNoOfBetaSteps() const {
	return m_noOfBetaSteps;
}
unsigned char BaseChR_Model::GetVerboseLevel() const {
	return m_verboseLevel;
}
std::vector<BaseChR_Model::PhysicsTableData>& BaseChR_Model::GetChRPhysDataVec() {
	return m_ChRPhysDataVec;
}
bool BaseChR_Model::GetUseModelWithEnergyLoss() const {
	return m_useModelWithEnergyLoss;
}

endChR

#endif // !BaseChR_Model_hpp