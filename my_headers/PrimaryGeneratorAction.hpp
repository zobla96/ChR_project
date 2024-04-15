#pragma once
#ifndef PrimaryGeneratorAction_hpp
#define PrimaryGeneratorAction_hpp

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
	inline double SetBeamSigma(const double);
	inline double SetParticleEnergy(const double);
	inline double SetDistanceZ(const double);
	inline unsigned int SetMassNo(const unsigned int);
	inline unsigned int SetAtomicNo(const unsigned int);
	inline unsigned int SetNoOfParticles(const unsigned int);
	inline void SetSigErrParameter(const double);
	inline void SetDivergenceSigma(const double);
	//=======Get inlines=======
	_NODISCARD inline double GetBeamSigma() const;
	_NODISCARD inline double GetDivSigma() const;
	_NODISCARD inline double GetParticleEnergy() const;
	_NODISCARD inline double GetDistanceZ() const; //it's an absolute value while it should be negative
	_NODISCARD inline unsigned int GetMassNo() const;
	_NODISCARD inline unsigned int GetAtomicNo() const;
	_NODISCARD inline unsigned int GetNoOfParticles() const;
	_NODISCARD inline const unsigned int& GetRefNoOfParticles() const;
	_NODISCARD inline G4ParticleGun* GetParticleGun() const;
	_NODISCARD inline const std::normal_distribution<double>& GetSigErrGauss() const;
private:
	double m_beamSigma;
	double m_energy; //full e- energy or for Z,A!=0 energy per nucleon
	double m_zDistance;
	unsigned int m_noOfParticles;
	unsigned int m_massNo : 9; //bit-field limit 511 [0,512)
	unsigned int m_atomicNo : 7/*, : 0*/; //bit-field limit 127 [0,128)
	unsigned short m_countdown;
	std::normal_distribution<double> m_sigBeamGauss;
	std::normal_distribution<double> m_sigErrGauss;
	std::normal_distribution<double> m_divergenceGauss;
	//For some reason, I've decided to build this class as I have
	//That means most of the /gun/ functionality is usless
	G4ParticleGun* p_theGun = nullptr;
	PrimaryGeneratorAction_Messenger* p_pGeneratorMessenger = nullptr;
};

//=======Set inlines=======
double PrimaryGeneratorAction::SetBeamSigma(const double value) {
	double temp = m_beamSigma;
	m_beamSigma = value;
	return temp;
}
double PrimaryGeneratorAction::SetParticleEnergy(const double value) {
	double temp = m_energy;
	m_energy = value;
	return temp;
}
double PrimaryGeneratorAction::SetDistanceZ(const double value) {
	double temp = m_zDistance;
	m_zDistance = value;
	return temp;
}
unsigned int PrimaryGeneratorAction::SetMassNo(const unsigned int value) {
	unsigned int temp = m_massNo;
	m_massNo = value;
	return temp;
}
unsigned int PrimaryGeneratorAction::SetAtomicNo(const unsigned int value) {
	unsigned int temp = m_atomicNo;
	m_atomicNo = value;
	return temp;
}
unsigned int PrimaryGeneratorAction::SetNoOfParticles(const unsigned int value) {
	unsigned int temp = m_noOfParticles;
	m_noOfParticles = value;
	return temp;
}
void PrimaryGeneratorAction::SetSigErrParameter(const double val) {
	m_sigErrGauss.param(std::normal_distribution<double>::param_type{0., val});
}
void PrimaryGeneratorAction::SetDivergenceSigma(const double val) {
	m_divergenceGauss.param(std::normal_distribution<double>::param_type{ 0., val});
}

//=======Get inlines=======
double PrimaryGeneratorAction::GetBeamSigma() const {
	return m_beamSigma;
}
double PrimaryGeneratorAction::GetDivSigma() const {
	return m_divergenceGauss.sigma();
}
double PrimaryGeneratorAction::GetParticleEnergy() const {
	return m_energy;
}
double PrimaryGeneratorAction::GetDistanceZ() const {
	return m_zDistance;
}
unsigned int PrimaryGeneratorAction::GetMassNo() const {
	return m_massNo;
}
unsigned int PrimaryGeneratorAction::GetAtomicNo() const {
	return m_atomicNo;
}
G4ParticleGun* PrimaryGeneratorAction::GetParticleGun() const {
	return p_theGun;
}
unsigned int PrimaryGeneratorAction::GetNoOfParticles() const {
	return m_noOfParticles;
}
const unsigned int& PrimaryGeneratorAction::GetRefNoOfParticles() const {
	return m_noOfParticles;
}
const std::normal_distribution<double>& PrimaryGeneratorAction::GetSigErrGauss() const {
	return m_sigErrGauss;
}

endChR
#endif // !PrimaryGeneratorAction_hpp