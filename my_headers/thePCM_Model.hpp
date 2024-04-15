#pragma once
#ifndef thePCM_Model_hpp
#define thePCM_Model_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "BaseChR_Model.hpp"
//std:: headers


beginChR

template<typename T>
class thePCM_Model : public BaseChR_Model {
public:
	static_assert(std::is_enum_v<T> == true, "The class thePCM_Model<T> is designed to support enum tpye of T!");
	thePCM_Model(const char* name = "thePCM_Model");
	virtual ~thePCM_Model();
	_NODISCARD virtual G4VParticleChange* PostStepModelDoIt(const G4Track&, const G4Step&) override;
	virtual void DumpModelInfo() const override;
private:

};

template<typename T>
thePCM_Model<T>::thePCM_Model(const char* name)
: BaseChR_Model(name) {
	m_includeFiniteThickness = true;
}

template<typename T>
thePCM_Model<T>::~thePCM_Model() {

}

template<typename T>
G4VParticleChange* thePCM_Model<T>::PostStepModelDoIt(const G4Track& aTrack, const G4Step& aStep) {


	return p_particleChange;
}

template<typename T>
void thePCM_Model<T>::DumpModelInfo() const {

}

endChR

#endif // !thePCM_Model_hpp