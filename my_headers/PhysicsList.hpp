#pragma once
#ifndef PhysicsList_hpp
#define PhysicsList_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "DefsNConsts.hpp"
#include "MyOpticalPhysics.hpp"
#include "PhysicsList_Messenger.hpp"
//G4 headers
#include "G4VModularPhysicsList.hh"
//...
#include "G4RegionStore.hh"
#include "G4StateManager.hh"
//Be careful, many physics classes don't specify G4BuilderType, i.e., bUnknown is used
//G4BuilderType::bUnknown
//#include "G4OpticalPhysics.hh"
//G4BuilderType::bElectromagnetic
//#include "G4EmDNAPhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4EmStandardPhysics_option1.hh"
#include "G4EmStandardPhysics_option2.hh"
#include "G4EmStandardPhysics_option3.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4EmLivermorePhysics.hh"
#include "G4EmLowEPPhysics.hh"
#include "G4EmPenelopePhysics.hh"
#include "G4EmStandardPhysicsGS.hh"
#include "G4EmStandardPhysicsSS.hh"
#include "G4EmStandardPhysicsWVI.hh"
//G4BuilderType::bEmExtra
#include "G4EmExtraPhysics.hh"
//G4BuilderType::bDecay
#include "G4DecayPhysics.hh"
//G4BuilderType::bHadronElastic
#include "G4HadronElasticPhysics.hh"
//G4BuilderType::bHadronInelastic
/*Using only the first one here as it's not essential for this project. Still, keep
in mind that I wrote only 4 such physics classes here, but there are many more... the ones
I wrote directly use G4BuilderType::bHadronInelastic while all other remain bUnknown*/
#include "G4HadronInelasticQBBC.hh"
//#include "G4HadronPhysicsFTFP_BERT.hh"
//#include "G4HadronPhysicsQGSP_BERT.hh"
//#include "G4HadronPhysicsQGSP_BIC.hh"
//G4BuilderType::bStopping
#include "G4StoppingPhysics.hh"
//#include "G4StoppingPhysicsFritiofWithBinaryCascade.hh"
//G4BuilderType::bIons
#include "G4IonPhysics.hh"
//#include "G4IonPhysicsPHP.hh"
//#include "G4IonQMDPhysics.hh"
//#include "G4IonINCLXXPhysics.hh"

beginChR

class PhysicsList_Messenger;

enum class UseElectromagnetic {
	G4EmStandardPhysics,
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

//=========ChR namespace operator=========
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
	default: //how??
		G4Exception("PhysicsList.hpp operator<<", "FE1018", FatalException, "Default output\n");
		break;
	}
	return outS;
}

class PhysicsList : public G4VModularPhysicsList {
public:
	PhysicsList(double gamma = -1., double electron = -1., double positron = -1., double proton = -1., int verbose = 1);
	virtual ~PhysicsList() override;
	/*Not overriding the following, i.e., using the standard methods
	which go through ConstructParticle and ConstructProcess for all
	registered G4VPhysicsConstructors*/
	//virtual void ConstructParticle() override; //If overriding, make sure to call "MyOpticalPhysics::ConstructParticle"!!
	//virtual void ConstructProcess() override;
	void SetCuts() override;
	void ChangeEMPhysics(UseElectromagnetic);
	//=======Set inlines=======
	inline double SetRadiatorRangeCuts_gamma(const double);
	inline double SetRadiatorRangeCuts_electron(const double);
	inline double SetRadiatorRangeCuts_positron(const double);
	inline double SetRadiatorRangeCuts_proton(const double);
	inline void SetUseNonDefaultCuts(const bool);
	//=======Get inlines=======
	_NODISCARD inline double GetRadiatorRangeCuts_gamma() const;
	_NODISCARD inline double GetRadiatorRangeCuts_electron() const;
	_NODISCARD inline double GetRadiatorRangeCuts_positron() const;
	_NODISCARD inline double GetRadiatorRangeCuts_proton() const;
	_NODISCARD inline bool GetUseNonDefaultCuts() const;
	_NODISCARD inline UseElectromagnetic GetEMPhysicsInUse() const;
private:
	bool m_useNonDefaultCuts;
	double m_radiatorRangeCuts_gamma;
	double m_radiatorRangeCuts_electron;
	double m_radiatorRangeCuts_positron;
	double m_radiatorRangeCuts_proton;
	UseElectromagnetic m_EMPhysics;
	G4VPhysicsConstructor* p_theEMPhysics = nullptr;
	PhysicsList_Messenger* p_phListMessenger = nullptr;
};

//=======Set inlines=======
double PhysicsList::SetRadiatorRangeCuts_gamma(const double value) {
	double temp = m_radiatorRangeCuts_gamma;
	m_radiatorRangeCuts_gamma = value;
	return temp;
}
double PhysicsList::SetRadiatorRangeCuts_electron(const double value) {
	double temp = m_radiatorRangeCuts_electron;
	m_radiatorRangeCuts_electron = value;
	return temp;
}
double PhysicsList::SetRadiatorRangeCuts_positron(const double value) {
	double temp = m_radiatorRangeCuts_positron;
	m_radiatorRangeCuts_positron = value;
	return temp;
}
double PhysicsList::SetRadiatorRangeCuts_proton(const double value) {
	double temp = m_radiatorRangeCuts_proton;
	m_radiatorRangeCuts_proton = value;
	return temp;
}
void PhysicsList::SetUseNonDefaultCuts(const bool value) {
	m_useNonDefaultCuts = value;
}

//=======Get inlines=======
double PhysicsList::GetRadiatorRangeCuts_gamma() const {
	return m_radiatorRangeCuts_gamma;
}
double PhysicsList::GetRadiatorRangeCuts_electron() const {
	return m_radiatorRangeCuts_electron;
}
double PhysicsList::GetRadiatorRangeCuts_positron() const {
	return m_radiatorRangeCuts_positron;
}
double PhysicsList::GetRadiatorRangeCuts_proton() const{
	return m_radiatorRangeCuts_proton;
}
bool PhysicsList::GetUseNonDefaultCuts() const {
	return m_useNonDefaultCuts;
}
UseElectromagnetic PhysicsList::GetEMPhysicsInUse() const {
	return m_EMPhysics;
}

endChR

#endif // !PhysicsList_hpp