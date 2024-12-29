//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef PrimaryGeneratorAction_hh
#define PrimaryGeneratorAction_hh

// user headers
#include "DefsNConsts.hh"
#include "PrimaryGeneratorAction_Messenger.hh"
// G4 headers
#include "G4VUserPrimaryGeneratorAction.hh"

class G4ParticleGun;

beginChR

class PrimaryGeneratorAction_Messenger;

class PrimaryGeneratorAction final : public G4VUserPrimaryGeneratorAction {
public:
	PrimaryGeneratorAction();
	~PrimaryGeneratorAction() override;
	void GeneratePrimaries(G4Event*) override;
	//=======Set inlines=======
	inline void SetBeamSigma(const G4double);
	inline void SetDistanceZ(const G4double);
	inline void SetDivergenceSigma(const G4double);
	//=======Get inlines=======
	[[nodiscard]] inline G4double GetBeamSigma() const;
	[[nodiscard]] inline G4double GetDivSigma() const;
	[[nodiscard]] inline G4double GetDistanceZ() const; // it's an absolute value while it should be negative
	[[nodiscard]] inline G4ParticleGun* GetParticleGun() const; // non-const gun for simplicity
private:
	G4ParticleGun* p_theGun = nullptr;
	PrimaryGeneratorAction_Messenger* p_pGeneratorMessenger = nullptr;
	G4double m_beamSigma;
	G4double m_sinBeamDivergenceTheta;
	G4double m_zDistance;
};

//=======Set inlines=======
void PrimaryGeneratorAction::SetBeamSigma(const G4double value) {
	m_beamSigma = value;
}
void PrimaryGeneratorAction::SetDistanceZ(const G4double value) {
	m_zDistance = value;
}
void PrimaryGeneratorAction::SetDivergenceSigma(const G4double val) {
	m_sinBeamDivergenceTheta = std::sin(val);
}

//=======Get inlines=======
G4double PrimaryGeneratorAction::GetBeamSigma() const {
	return m_beamSigma;
}
G4double PrimaryGeneratorAction::GetDivSigma() const {
	return std::asin(m_sinBeamDivergenceTheta);
}
G4double PrimaryGeneratorAction::GetDistanceZ() const {
	return m_zDistance;
}
G4ParticleGun* PrimaryGeneratorAction::GetParticleGun() const {
	return p_theGun;
}

endChR

#endif // !PrimaryGeneratorAction_hh