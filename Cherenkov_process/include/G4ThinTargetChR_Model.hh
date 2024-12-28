//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

/*
ABOUT THE HEADER
----------------

Read the "DumpModelInfo" method
*/

#pragma once
#ifndef G4ThinTargetChR_Model_hh
#define G4ThinTargetChR_Model_hh

#include "G4BaseChR_Model.hh"
#include "G4UImessenger.hh"

class G4ThinTargetChR_Model;
class G4UIcommand;
class G4UIcmdWithADouble;

class G4ThinTargetChR_ModelMessenger : public G4UImessenger {
public:
	G4ThinTargetChR_ModelMessenger(G4ThinTargetChR_Model*);
	virtual ~G4ThinTargetChR_ModelMessenger();
	virtual void SetNewValue(G4UIcommand* uiCmd, G4String aStr) override;
private:
	G4ThinTargetChR_Model* p_thinChRTarget = nullptr;
	G4UIcmdWithADouble* p_multiplierCoef = nullptr;
};

class G4ThinTargetChR_Model : public G4BaseChR_Model {
public:
	G4ThinTargetChR_Model(const char* theName = "G4ThinTargetChR_Model");
	virtual ~G4ThinTargetChR_Model() override = default;

	G4ThinTargetChR_Model(const G4ThinTargetChR_Model&) = delete;
	G4ThinTargetChR_Model& operator=(const G4ThinTargetChR_Model&) = delete;
	G4ThinTargetChR_Model(G4ThinTargetChR_Model&&) /*noexcept*/ = delete;
	G4ThinTargetChR_Model& operator=(G4ThinTargetChR_Model&&) /*noexcept*/ = delete;

	[[nodiscard]] virtual G4VParticleChange* PostStepModelDoIt(const G4Track&, const G4Step&, const G4CherenkovMatData&) override;
	virtual void DumpModelInfo() const override;

	//=======Set inlines=======
	inline void SetMultiplierCoef(const G4double);

	//=======Get inlines=======
	inline G4double GetMultiplierCoef() const;
private:
	G4double m_coef; // determines (multiplies) widening of the lateral surface for thin targets
	std::unique_ptr<G4ThinTargetChR_ModelMessenger> m_theMessenger;
};

//=======Set inlines=======
void G4ThinTargetChR_Model::SetMultiplierCoef(const G4double aValue) {
	m_coef = aValue;
}

//=======Get inlines=======
G4double G4ThinTargetChR_Model::GetMultiplierCoef() const {
	return m_coef;
}

#endif // !G4ThinTargetChR_Model_hh