#pragma once
#ifndef MyOpticalParameters_Messenger_hpp
#define MyOpticalParameters_Messenger_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "MyOpticalParameters.hpp"
//G4 headers
#include "G4UImessenger.hh"
//...
#include "G4UIdirectory.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"
//std:: headers

beginChR

template <typename T>
class MyOpticalParameters;

template<typename T>
class MyOpticalParameters_Messenger final : public G4UImessenger {
public:
	explicit MyOpticalParameters_Messenger(MyOpticalParameters<T>*);
	virtual ~MyOpticalParameters_Messenger() override;
	virtual void SetNewValue(G4UIcommand*, G4String) override;
private:
	MyOpticalParameters<T>* p_myOpticalParameters = nullptr;
	G4UIdirectory* p_opticalParametersDIR = nullptr;
	G4UIcmdWithABool* p_stronglyForcedThinTarget = nullptr;
	G4UIcmdWithADoubleAndUnit* p_thinTargetLimit = nullptr;
	G4UIcmdWithAnInteger* p_verboseLevel = nullptr;
};

//=========public ChR::MyOpticalParameters_Messenger<T>:: methods=========
template <typename T>
MyOpticalParameters_Messenger<T>::MyOpticalParameters_Messenger(MyOpticalParameters<T>* optParams)
:p_myOpticalParameters(optParams) {
	p_opticalParametersDIR = new G4UIdirectory("/ChR_project/OpticalParameters/");
	p_opticalParametersDIR->SetGuidance("All commands related to MyOpticalParameters<T>");

	p_stronglyForcedThinTarget = new G4UIcmdWithABool("/ChR_project/OpticalParameters/stronglyForcedThinTarget", this);
	p_stronglyForcedThinTarget->SetGuidance("Used to change stronglyForcedThinTarget condition.");
	p_stronglyForcedThinTarget->SetGuidance("This condition is used if one registers his/her own enum type for Cherenkov process and doesn't specialize functions.");
	p_stronglyForcedThinTarget->SetGuidance("If ChR::ChRModelIndex is used, this parameters is useless (e.g., in ChR_project)!");
	p_stronglyForcedThinTarget->SetParameterName("forcedThinFlag", true);
	p_stronglyForcedThinTarget->SetDefaultValue(false);
	p_stronglyForcedThinTarget->AvailableForStates(G4State_PreInit, G4State_Idle); //to be used in .mac files

	p_thinTargetLimit = new G4UIcmdWithADoubleAndUnit("/ChR_project/OpticalParameters/thinTargetLimit", this);
	p_thinTargetLimit->SetGuidance("Used to change limit for applying thin target models.");
	p_thinTargetLimit->SetGuidance("If the set-material thickness is greater than this limit, a standard Cherenkov model will be used.");
	p_thinTargetLimit->SetGuidance("This condition is used if one registers his/her own enum type for Cherenkov process and doesn't specialize functions.");
	p_thinTargetLimit->SetGuidance("If ChR::ChRModelIndex is used, this parameters is useless (e.g., in ChR_project)!");
	p_thinTargetLimit->SetParameterName("thinTargetLimit", true);
	p_thinTargetLimit->SetDefaultUnit("um");
	p_thinTargetLimit->SetDefaultValue(1000.);
	p_thinTargetLimit->SetRange("thinTargetLimit>0");
	p_thinTargetLimit->AvailableForStates(G4State_PreInit, G4State_Idle);

	p_verboseLevel = new G4UIcmdWithAnInteger("/ChR_project/OpticalParameters/verboseLevel", this);
	p_verboseLevel->SetGuidance("Used to change verboseLevel of the MyOpticalParameters<T> class.");
	p_verboseLevel->SetGuidance("Currently, the only use of this value is to pass it to G4OpticalParameters.");
	p_verboseLevel->SetParameterName("verboseLevel", true);
	p_verboseLevel->SetDefaultValue(0);
	p_verboseLevel->SetRange("verboseLevel>=0");
	p_verboseLevel->AvailableForStates(G4State_PreInit, G4State_Idle);
}

template <typename T>
MyOpticalParameters_Messenger<T>::~MyOpticalParameters_Messenger() {
	delete p_opticalParametersDIR;
	delete p_stronglyForcedThinTarget;
	delete p_thinTargetLimit;
	delete p_verboseLevel;
}

template <typename T>
void MyOpticalParameters_Messenger<T>::SetNewValue(G4UIcommand* uiCmd, G4String aStr) {
	if (uiCmd == p_stronglyForcedThinTarget) {
		p_myOpticalParameters->SetForcedThinTargetCondition(p_stronglyForcedThinTarget->ConvertToBool(aStr));
	}
	else if (uiCmd == p_thinTargetLimit) {
		p_myOpticalParameters->SetThinTargetLimit(p_thinTargetLimit->GetNewDoubleValue(aStr));
	}
	else if (uiCmd == p_verboseLevel) {
		p_myOpticalParameters->SetVerboseLevel(p_verboseLevel->ConvertToInt(aStr));
	}
	else
		G4Exception("MyOpticalParameters_Messenger<T>::SetNewValue", "WE1020", JustWarning, "Command not found!\n");
}

endChR

#endif // !MyOpticalParameters_Messenger_hpp