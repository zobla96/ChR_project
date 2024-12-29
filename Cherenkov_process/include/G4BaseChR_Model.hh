//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

/*
ABOUT THE HEADER
----------------

The class G4BaseChR_Model must be inherited by all Cherenkov models that
are registered in the G4CherenkovProcess class. As all other processes,
this class contains PostStepInteractionLength and PostStepDoIt (according
to G4VDiscreteProcess). However, in the current version, the method
PostStepInteractionLength is not pure virtual as I assume that all/most
models can use the same method. On the other hand, the latter method is
pure virtual and one should decide what kind of angular and spectral
distributions should be used.

The number of photons is generated according to equation:
dN/dl = 2 * Pi * alpha * z^2 / (h * c) * (leftIntegral - rightIntegral),
where:
leftIntegral = \int dE
rightIntegral = \int dE / (beta^2 * RIndex^2)
with limits: Emin -> Emax

The physics lists of the G4BaseChR_Model are prepared to analyze any kind of
RIndex dependency and are saved as a 'std::vector<G4ChRPhysTableData>',
where 'G4ChRPhysTableData' is a helper struct containing values based around
specific beta values:
1. beta values - specific beta values that are used when a charged particle
                 cannot emit Cherenkov photons on the whole RIndex spectrum
2. left integral value (dE)
3. right integral value (dE / RIndex^2)
4. p_valuesCDF - to keep normalized CDF values used to generate spectral
				 distribution of ChR for low beta values. The value is 'nullptr'
				 if non-exotic refractive indices are used

And a vector of values for building CDFs for particles with beta values higher
than betaMax, if a material has an exotic refractive index:
5. p_bigBetaCDFVector - the value is mostly 'nullptr'. It is used for materials
						with exotic refractive indices, and beta values greater
						than the betaMax value (the value that can generate ChR
						photons on the whole RIndex spectrum)

Note that the ChR currently doesn't produce ChR photons in X-ray region (gammas).
Also, the whole idea with the spectral distribution of exotic materials  is based
on the theory and it still must be confirmed by an experiment.

Some other member variables:
1. m_noOfBetaSteps - for filling the beta-values vector from betaMin to
                     betaMax with "m_noOfBetaStep + 1" nodes
2. m_includeFiniteThickness - Cherenkov models should mark if they can
                     consider thin radiators adequately (currently not used,
					 but it could be used for some safety conditions)
3. m_useModelWithEnergyLoss - set to true to include conservation energy law
                     (according to Tamm-Frank theory, it's not satisfied; the
					 values are negligible)

Other than member variables, most virtual methods are directly related to the
G4VProcess virtual methods.
*/

#pragma once
#ifndef G4BaseChR_Model_hh
#define G4BaseChR_Model_hh

//G4 headers
#include "G4ForceCondition.hh"
#include "G4ChRPhysicsTableData.hh"

class G4Track;
class G4VParticleChange;
class G4ParticleChange;
class G4ParticleDefinition;
class G4Step;
class G4Material;
class G4ExtraOpticalParameters_Messenger;
struct G4CherenkovMatData;

class G4BaseChR_Model {
	friend G4ExtraOpticalParameters_Messenger;
public:
	using G4ChRPhysicsTableVector = std::vector<G4ChRPhysTableData>;

	G4BaseChR_Model(const char* name, const unsigned char verboseLevel = 0);
	virtual ~G4BaseChR_Model();
	G4BaseChR_Model(const G4BaseChR_Model&) = delete;
	G4BaseChR_Model& operator=(const G4BaseChR_Model&) = delete;
	G4BaseChR_Model(G4BaseChR_Model&&) /*noexcept*/ = delete;
	G4BaseChR_Model& operator=(G4BaseChR_Model&&) /*noexcept*/ = delete;

	//=======Methods according G4VProcess=======
	[[nodiscard]] virtual G4double PostStepModelIntLength(const G4Track& aTrack, G4double previousStepSize, G4ForceCondition* condition);

	virtual G4VParticleChange* PostStepModelDoIt(const G4Track& aTrack, const G4Step& aStep, const G4CherenkovMatData& selectedData) = 0;

	virtual inline void PrepareModelPhysicsTable(const G4ParticleDefinition&) {};
	virtual inline void BuildModelPhysicsTable(const G4ParticleDefinition&);

	//it might be good to load binaries for some more complex physics tables
	virtual inline G4bool StoreModelPhysicsTable(const G4ParticleDefinition*, const G4String&, G4bool) { return true; }
	virtual inline G4bool RetrieveModelPhysicsTable(const G4ParticleDefinition*, const G4String&, G4bool) { return true; }

	virtual inline void PrepareWorkerModelPhysicsTable(const G4ParticleDefinition& aParticle);
	virtual inline void BuildWorkerModelPhysicsTable(const G4ParticleDefinition& aParticle);

	virtual inline void DumpModelInfo() const;
	static void PrintChRPhysDataVec(const unsigned char printLevel = 0,  const G4Material* aMaterial = nullptr);
	// aMaterial == nullptr -> prints physics tables for all registered materials
	// aMaterial == someMaterial -> prints physics tables for a someMaterial
	// printLevel == 0 -> print only basic available information about registered physics tables
	// printLevel == 1 -> print standard + p_valuesCDF values
	// printLevel >= 2 -> print all available information about registered physics tables

	//=======Set inlines=======
	inline static unsigned int SetNoOfBetaSteps(const unsigned int);
	inline void SetVerboseLevel(const unsigned char);
	inline void SetUseModelWithEnergyLoss(const G4bool);

	//=======Get inlines=======
	[[nodiscard]] inline const char* GetChRModelName() const;
	[[nodiscard]] inline static unsigned int GetNoOfBetaSteps();
	[[nodiscard]] inline unsigned char GetVerboseLevel() const;
	[[nodiscard]] inline G4bool GetFiniteThicknessCondition() const;
	[[nodiscard]] inline G4bool GetUseModelWithEnergyLoss() const;
	[[nodiscard]] inline const static G4ChRPhysicsTableVector& GetChRPhysDataVec();

protected:
	[[nodiscard]] virtual G4double CalculateAverageNumberOfPhotons(const G4double aCharge, const G4double betaValue, const size_t materialID);

	static G4ChRPhysicsTableVector m_ChRPhysDataVec;
	static unsigned int m_noOfBetaSteps;
	//=======Member variables=======
	G4ParticleChange* p_particleChange = nullptr;
	const char* m_ChRModelName;
	unsigned char m_verboseLevel;
	G4bool m_includeFiniteThickness;
	G4bool m_useModelWithEnergyLoss;
	// 5 wasted bytes on x64
	//==============================

private:
	G4BaseChR_Model() = delete;
	// the following method tries, but adds physics table with exotic RI or not. Nevertheless, they primarily use exotic if possible
	static G4bool AddExoticRIndexPhysicsTable(const size_t materialID, G4bool forceExoticFlag = false);
	static void RemoveExoticRIndexPhysicsTable(const size_t materialID);
};

//=======Set inlines=======
unsigned int G4BaseChR_Model::SetNoOfBetaSteps(const unsigned int value) {
	unsigned int temp = m_noOfBetaSteps;
	m_noOfBetaSteps = value;
	return temp;
}
void G4BaseChR_Model::SetVerboseLevel(const unsigned char value) {
	m_verboseLevel = value;
}
void G4BaseChR_Model::SetUseModelWithEnergyLoss(const G4bool value) {
	m_useModelWithEnergyLoss = value;
}

//=======Get inlines=======
const char* G4BaseChR_Model::GetChRModelName() const {
	return m_ChRModelName;
}
unsigned int G4BaseChR_Model::GetNoOfBetaSteps() {
	return m_noOfBetaSteps;
}
unsigned char G4BaseChR_Model::GetVerboseLevel() const {
	return m_verboseLevel;
}
G4bool G4BaseChR_Model::GetFiniteThicknessCondition() const {
	return m_includeFiniteThickness;
}
G4bool G4BaseChR_Model::GetUseModelWithEnergyLoss() const {
	return m_useModelWithEnergyLoss;
}
const G4BaseChR_Model::G4ChRPhysicsTableVector& G4BaseChR_Model::GetChRPhysDataVec() {
	return m_ChRPhysDataVec;
}

//=======Additional inlines=======
void G4BaseChR_Model::PrepareWorkerModelPhysicsTable(const G4ParticleDefinition& aParticle) {
	PrepareModelPhysicsTable(aParticle);
}

void G4BaseChR_Model::BuildWorkerModelPhysicsTable(const G4ParticleDefinition& aParticle) {
	BuildModelPhysicsTable(aParticle);
}

void G4BaseChR_Model::DumpModelInfo() const {
	std::cout << "There's no information about the model!\n";
}

#endif // !G4BaseChR_Model_hh