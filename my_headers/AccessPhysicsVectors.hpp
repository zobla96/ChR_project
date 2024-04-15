#pragma once
#ifndef AccessPhysicsVector_hpp
#define AccessPhysicsVector_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "DefsNConsts.hpp"
//G4 headers
#include "G4PhysicsVector.hh"
//std:: headers
#include <algorithm>

beginChR

/*
Unfortunately, G4PhysicsFreeVector doesn't have a virtual destructor
and thus I don't want to inherit it... though in this case it can be done.
On the other hand, the problem here can be solved by casting (reinterpret) which will be done
*/
class AccessPhysicsVector : public G4PhysicsVector {
public:
	AccessPhysicsVector() = default;
	virtual ~AccessPhysicsVector() override = default;
	//=======Get inlines=======
	_NODISCARD inline std::vector<double> GetBinVector() const; //energy vector
	_NODISCARD inline std::vector<double> GetDataVector() const; //crossection/energyloss
	_NODISCARD inline std::vector<double> GetSecDerivative() const; //second derivatives
	/*
	This class was created just to access vectors and write the following functions
	In the original class, instead of getting a real max/min, only the left/right-most value
	is returned. That means that if n(E) spectrum is a more complex function, one can't see it
	In general, it would be better to call functions GetFrontValue() and GetBackValue(), not
	GetMinValue() and GetMaxValue(). That way there would be no confusion for sure
	*/
	_NODISCARD inline double GetRealDataVectorMax() const;
	_NODISCARD inline double GetRealDataVectorMin() const;
	inline void PushBackData(const std::pair<double, double>&);
	inline void PushBackData(const double, const double);
};

//=======Get inlines=======
std::vector<double> AccessPhysicsVector::GetBinVector() const {
	return binVector;
}
std::vector<double> AccessPhysicsVector::GetDataVector() const {
	return dataVector;
}
std::vector<double> AccessPhysicsVector::GetSecDerivative() const {
	return secDerivative;
}
double AccessPhysicsVector::GetRealDataVectorMax() const {
	return dataVector.at(std::distance(dataVector.begin(), std::max_element(dataVector.begin(), dataVector.end())));
}
double AccessPhysicsVector::GetRealDataVectorMin() const {
	return dataVector.at(std::distance(dataVector.begin(), std::min_element(dataVector.begin(), dataVector.end())));
}
void AccessPhysicsVector::PushBackData(const std::pair<double, double>& aPair) {
	binVector.push_back(aPair.first);
	dataVector.push_back(aPair.second);
}
void AccessPhysicsVector::PushBackData(const double val1, const double val2) {
	binVector.push_back(val1);
	dataVector.push_back(val2);
}

endChR

#endif // !AccessPhysicsVector_hpp