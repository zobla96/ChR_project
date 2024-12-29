//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef PhysicsList_hh
#define PhysicsList_hh

//User built headers
#include "DefsNConsts.hh"
#include "PhysicsList_Messenger.hh"
//G4 headers
#include "G4VModularPhysicsList.hh"

class G4VPhysicsConstructor;

beginChR

class PhysicsList_Messenger;

enum class UseElectromagnetic {
	G4EmStandardPhysics = 0,
	G4EmStandardPhysics_option1,
	G4EmStandardPhysics_option2,
	G4EmStandardPhysics_option3,
	G4EmStandardPhysics_option4,
	G4EmLivermorePhysics,
	G4EmLowEPPhysics,
	G4EmPenelopePhysics,
	G4EmStandardPhysicsGS,
	G4EmStandardPhysicsSS,
	G4EmStandardPhysicsWVI
};

enum class UseOptical {
	G4OpticalPhysics = 0,
	G4OpticalPhysics_option1,
	G4OpticalPhysics_option2
};

//=========ChR namespace operators=========
inline std::ostream& operator<<(std::ostream& outS, const UseElectromagnetic& theEMPhys) {
	switch (theEMPhys) {
	case ChR::UseElectromagnetic::G4EmStandardPhysics:
		outS << "G4EmStandardPhysics";
		break;
	case ChR::UseElectromagnetic::G4EmStandardPhysics_option1:
		outS << "G4EmStandardPhysics_option1";
		break;
	case ChR::UseElectromagnetic::G4EmStandardPhysics_option2:
		outS << "G4EmStandardPhysics_option2";
		break;
	case ChR::UseElectromagnetic::G4EmStandardPhysics_option3:
		outS << "G4EmStandardPhysics_option3";
		break;
	case ChR::UseElectromagnetic::G4EmStandardPhysics_option4:
		outS << "G4EmStandardPhysics_option4";
		break;
	case ChR::UseElectromagnetic::G4EmLivermorePhysics:
		outS << "G4EmLivermorePhysics";
		break;
	case ChR::UseElectromagnetic::G4EmLowEPPhysics:
		outS << "G4EmLowEPPhysics";
		break;
	case ChR::UseElectromagnetic::G4EmPenelopePhysics:
		outS << "G4EmPenelopePhysics";
		break;
	case ChR::UseElectromagnetic::G4EmStandardPhysicsGS:
		outS << "G4EmStandardPhysicsGS";
		break;
	case ChR::UseElectromagnetic::G4EmStandardPhysicsSS:
		outS << "G4EmStandardPhysicsSS";
		break;
	case ChR::UseElectromagnetic::G4EmStandardPhysicsWVI:
		outS << "G4EmStandardPhysicsWVI";
		break;
	default: // how??
		G4Exception("PhysicsList.hpp operator<<", "FE_PhysList05", FatalException, "Default output\n");
		break;
	}
	return outS;
}

inline std::ostream& operator<<(std::ostream& outS, const UseOptical& theEMPhys) {
	switch (theEMPhys) {
	case ChR::UseOptical::G4OpticalPhysics:
		outS << "G4OpticalPhysics";
		break;
	case ChR::UseOptical::G4OpticalPhysics_option1:
		outS << "G4OpticalPhysics_option1";
		break;
	case ChR::UseOptical::G4OpticalPhysics_option2:
		outS << "G4OpticalPhysics_option2";
		break;
	default: // how??
		G4Exception("PhysicsList.hpp operator<<", "FE_PhysList06", FatalException, "Default output\n");
		break;
	}
	return outS;
}

class PhysicsList : public G4VModularPhysicsList {
public:
	PhysicsList(G4int verbose = 1, G4double gamma = -1., G4double electron = -1., G4double positron = -1., G4double proton = -1.);
	virtual ~PhysicsList() override;
	/*Not overriding the following, i.e., using the standard methods
	which go through ConstructParticle and ConstructProcess for all
	registered G4VPhysicsConstructors*/
	//virtual void ConstructParticle() override;
	//virtual void ConstructProcess() override;
	virtual void SetCuts() override;
	void ChangeEMPhysics(const UseElectromagnetic);
	void ChangeOpticalPhysics(const UseOptical);
	//=======Set inlines=======
	inline void SetRadiatorRangeCuts_gamma(const G4double);
	inline void SetRadiatorRangeCuts_electron(const G4double);
	inline void SetRadiatorRangeCuts_positron(const G4double);
	inline void SetRadiatorRangeCuts_proton(const G4double);
	//=======Get inlines=======
	[[nodiscard]] inline G4double GetRadiatorRangeCuts_gamma() const;
	[[nodiscard]] inline G4double GetRadiatorRangeCuts_electron() const;
	[[nodiscard]] inline G4double GetRadiatorRangeCuts_positron() const;
	[[nodiscard]] inline G4double GetRadiatorRangeCuts_proton() const;
	[[nodiscard]] inline UseElectromagnetic GetEMPhysicsInUse() const;
	[[nodiscard]] inline UseOptical GetOpticalPhysicsInUse() const;
private:
	G4VPhysicsConstructor* p_theEMPhysics = nullptr;
	G4VPhysicsConstructor* p_opticalPhysics = nullptr;
	PhysicsList_Messenger* p_phListMessenger = nullptr;
	G4double m_radiatorRangeCuts_gamma;
	G4double m_radiatorRangeCuts_electron;
	G4double m_radiatorRangeCuts_positron;
	G4double m_radiatorRangeCuts_proton;
	UseElectromagnetic m_EMPhysics;
	UseOptical m_optical;
};

//=======Set inlines=======
void PhysicsList::SetRadiatorRangeCuts_gamma(const G4double value) {
	m_radiatorRangeCuts_gamma = value;
}
void PhysicsList::SetRadiatorRangeCuts_electron(const G4double value) {
	m_radiatorRangeCuts_electron = value;
}
void PhysicsList::SetRadiatorRangeCuts_positron(const G4double value) {
	m_radiatorRangeCuts_positron = value;
}
void PhysicsList::SetRadiatorRangeCuts_proton(const G4double value) {
	m_radiatorRangeCuts_proton = value;
}

//=======Get inlines=======
G4double PhysicsList::GetRadiatorRangeCuts_gamma() const {
	return m_radiatorRangeCuts_gamma;
}
G4double PhysicsList::GetRadiatorRangeCuts_electron() const {
	return m_radiatorRangeCuts_electron;
}
G4double PhysicsList::GetRadiatorRangeCuts_positron() const {
	return m_radiatorRangeCuts_positron;
}
G4double PhysicsList::GetRadiatorRangeCuts_proton() const {
	return m_radiatorRangeCuts_proton;
}
UseElectromagnetic PhysicsList::GetEMPhysicsInUse() const {
	return m_EMPhysics;
}
UseOptical PhysicsList::GetOpticalPhysicsInUse() const {
	return m_optical;
}

endChR

#endif // !PhysicsList_hh