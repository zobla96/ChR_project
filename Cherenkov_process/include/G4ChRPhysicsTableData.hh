//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef G4ChRPhysicsTableData_hh
#define G4ChRPhysicsTableData_hh

//G4 headers
#include "globals.hh"
#include "G4ThreeVector.hh"
//std:: headers
#include <vector>
#include <utility>

struct G4ChRPhysTableData {
	struct G4AroundBetaBasedValues;
	// going twice with std::vector<...>* as it should take less memory in most of applications, i.e.,
	// mostly CDF values are not needed! A vector itself is 24 bytes.
	std::vector<G4AroundBetaBasedValues> m_aroundBetaValues;
	std::vector<G4ThreeVector>* p_bigBetaCDFVector = nullptr; // leftInt and rightInt values for CDF, but beta is not included yet
	//used only for maxBeta

	struct G4AroundBetaBasedValues {
		G4AroundBetaBasedValues(G4double beta, G4double leftInt, G4double rightInt)
		 : m_betaValue(beta), m_leftIntegralValue(leftInt), m_rightIntegralValue(rightInt) {}
		~G4AroundBetaBasedValues() {
			delete p_valuesCDF;
		}
		G4double m_betaValue;
		G4double m_leftIntegralValue;
		G4double m_rightIntegralValue;
		std::vector<std::pair<G4double, G4double>>* p_valuesCDF = nullptr; // real cdf values in range [0, 1]
	};

	G4ChRPhysTableData() = default;
	~G4ChRPhysTableData() {
		delete p_bigBetaCDFVector;
	}

	G4ChRPhysTableData& operator=(const G4ChRPhysTableData& other) = delete;
	G4ChRPhysTableData(const G4ChRPhysTableData& other) = delete;

	G4ChRPhysTableData& operator=(G4ChRPhysTableData&& other) noexcept {
		if (this == &other)
			return *this;
		m_aroundBetaValues = std::exchange(other.m_aroundBetaValues, std::vector<G4AroundBetaBasedValues>{});
		delete p_bigBetaCDFVector;
		p_bigBetaCDFVector = std::exchange(other.p_bigBetaCDFVector, nullptr);
		return *this;
	}

	G4ChRPhysTableData(G4ChRPhysTableData&& other) noexcept
	: m_aroundBetaValues(std::exchange(other.m_aroundBetaValues, std::vector<G4AroundBetaBasedValues>{})),
	p_bigBetaCDFVector(std::exchange(other.p_bigBetaCDFVector, nullptr)) {}
};

#endif // !G4ChRPhysicsTableData_hh