//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef PrimaryGeneratorAction_hpp
#define PrimaryGeneratorAction_hpp

//User built headers
#include "DefsNConsts.hpp"
#include "UnitsAndBench.hpp"
#include "PrimaryGeneratorAction_Messenger.hpp"
//G4 headers
#include "G4VUserPrimaryGeneratorAction.hh"
//...
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "Randomize.hh"

beginChR

class PrimaryGeneratorAction_Messenger;

class PrimaryGeneratorAction final : public G4VUserPrimaryGeneratorAction {
	PrimaryGeneratorAction();
public:
	static PrimaryGeneratorAction* GetInstance();
	~PrimaryGeneratorAction() override;
	void GeneratePrimaries(G4Event*) override;
	//=======Set inlines=======
	inline void SetBeamSigma(const double);
	inline void SetParticleEnergy(const double);
	inline void SetDistanceZ(const double);
	inline void SetMassNo(const unsigned short);
	inline void SetAtomicNo(const unsigned short);
	inline void SetNoOfParticles(const int);
	inline void SetDivergenceSigma(const double);
	//=======Get inlines=======
	[[nodiscard]] inline double GetBeamSigma() const;
	[[nodiscard]] inline double GetDivSigma() const;
	[[nodiscard]] inline double GetParticleEnergy() const;
	[[nodiscard]] inline double GetDistanceZ() const; //it's an absolute value while it should be negative
	[[nodiscard]] inline unsigned short GetMassNo() const;
	[[nodiscard]] inline unsigned short GetAtomicNo() const;
	[[nodiscard]] inline int GetNoOfParticles() const;
	[[nodiscard]] inline const unsigned int& GetRefNoOfParticles() const;
	[[nodiscard]] inline G4ParticleGun* GetParticleGun() const;
private:
	double m_beamSigma;
	double m_sinBeamDivergenceTheta;
	//For some reason, I've decided to build this class as I have
	//That means most of the /gun/ functionality is useless
	//and I will waste a few bytes
	G4ParticleGun* p_theGun = nullptr;
	PrimaryGeneratorAction_Messenger* p_pGeneratorMessenger = nullptr;

	double m_energy; //full e- energy or for Z,A!=0 energy per nucleon
	double m_zDistance;
	unsigned int m_noOfParticles;
	// removed bit-fields -> 2 bytes would be wasted and bit-fields are slightly slower
	unsigned short m_massNo/* : 9*/; //bit-field limit 511 [0,512)
	unsigned short m_atomicNo/* : 7*//*, : 0*/; //bit-field limit 127 [0,128)
};

//=======Set inlines=======
void PrimaryGeneratorAction::SetBeamSigma(const double value) {
	m_beamSigma = value;
}
void PrimaryGeneratorAction::SetParticleEnergy(const double value) {
	m_energy = value;
}
void PrimaryGeneratorAction::SetDistanceZ(const double value) {
	m_zDistance = value;
}
void PrimaryGeneratorAction::SetMassNo(const unsigned short value) {
	m_massNo = value;
}
void PrimaryGeneratorAction::SetAtomicNo(const unsigned short value) {
	m_atomicNo = value;
}
void PrimaryGeneratorAction::SetNoOfParticles(const int value) {
	m_noOfParticles = value;
}
void PrimaryGeneratorAction::SetDivergenceSigma(const double val) {
	m_sinBeamDivergenceTheta = std::sin(val);
}

//=======Get inlines=======
double PrimaryGeneratorAction::GetBeamSigma() const {
	return m_beamSigma;
}
double PrimaryGeneratorAction::GetDivSigma() const {
	return std::asin(m_sinBeamDivergenceTheta);
}
double PrimaryGeneratorAction::GetParticleEnergy() const {
	return m_energy;
}
double PrimaryGeneratorAction::GetDistanceZ() const {
	return m_zDistance;
}
unsigned short PrimaryGeneratorAction::GetMassNo() const {
	return m_massNo;
}
unsigned short PrimaryGeneratorAction::GetAtomicNo() const {
	return m_atomicNo;
}
G4ParticleGun* PrimaryGeneratorAction::GetParticleGun() const {
	return p_theGun;
}
int PrimaryGeneratorAction::GetNoOfParticles() const {
	return m_noOfParticles;
}
const unsigned int& PrimaryGeneratorAction::GetRefNoOfParticles() const {
	return m_noOfParticles;
}

endChR

#endif // !PrimaryGeneratorAction_hpp