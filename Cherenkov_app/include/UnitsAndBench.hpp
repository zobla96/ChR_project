//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef UnitsAndBench_hpp
#define UnitsAndBench_hpp

//User built headers
#include "DefsNConsts.hpp"
//G4 headers
#include "G4SystemOfUnits.hh"
//std:: headers
#include <iostream>

beginChR

namespace myLiterals {
	////Note: to use this, one must use floating_point, I didn't overload for ints (or anything else)
	////Length
	//millimeter = 1.
	constexpr long double operator""_millimeter(long double rhv) { return rhv; }
	constexpr long double operator""_millimeter2(long double rhv) { return rhv; }
	constexpr long double operator""_millimeter3(long double rhv) { return rhv; }
	constexpr long double operator""_mm(long double rhv) { return rhv; }
	constexpr long double operator""_mm2(long double rhv) { return rhv; }
	constexpr long double operator""_mm3(long double rhv) { return rhv; }
	//centi = * 10. mm
	constexpr long double operator""_centimeter(long double rhv) { return rhv * cm; }
	constexpr long double operator""_centimeter2(long double rhv) { return rhv * cm2; }
	constexpr long double operator""_centimeter3(long double rhv) { return rhv * cm3; }
	constexpr long double operator""_cm(long double rhv) { return rhv * cm; }
	constexpr long double operator""_cm2(long double rhv) { return rhv * cm2; }
	constexpr long double operator""_cm3(long double rhv) { return rhv * cm3; }
	//deci = * 100. mm
	constexpr long double operator""_decimeter(long double rhv) { return rhv * 1.e2; }
	constexpr long double operator""_decimeter2(long double rhv) { return rhv * 1.e4; }
	constexpr long double operator""_decimeter3(long double rhv) { return rhv * 1.e6; }
	constexpr long double operator""_dm(long double rhv) { return rhv * 1.e2; }
	constexpr long double operator""_dm2(long double rhv) { return rhv * 1.e4; }
	constexpr long double operator""_dm3(long double rhv) { return rhv * 1.e6; }
	//Si unit = * 1000. mm
	constexpr long double operator""_meter(long double rhv) { return rhv * m; }
	constexpr long double operator""_meter2(long double rhv) { return rhv * m2; }
	constexpr long double operator""_meter3(long double rhv) { return rhv * m3; }
	constexpr long double operator""_m(long double rhv) { return rhv * m; }
	constexpr long double operator""_m2(long double rhv) { return rhv * m2; }
	constexpr long double operator""_m3(long double rhv) { return rhv * m3; }
	//kilo = * e+6 mm
	constexpr long double operator""_kilometer(long double rhv) { return rhv * km; }
	constexpr long double operator""_kilometer2(long double rhv) { return rhv * km2; }
	constexpr long double operator""_kilometer3(long double rhv) { return rhv * km3; }
	constexpr long double operator""_km(long double rhv) { return rhv * km; }
	constexpr long double operator""_km2(long double rhv) { return rhv * km2; }
	constexpr long double operator""_km3(long double rhv) { return rhv * km3; }
	//micro = * e-3 mm
	constexpr long double operator""_micrometer(long double rhv) { return rhv * um; }
	constexpr long double operator""_um(long double rhv) { return rhv * um; }
	//nano = * e-6 mm
	constexpr long double operator""_nanometer(long double rhv) { return rhv * nm; }
	constexpr long double operator""_nm(long double rhv) { return rhv * nm; }
	//pico = * e-9 mm
	constexpr long double operator""_picometer(long double rhv) { return rhv * 1.e-9; }
	constexpr long double operator""_pm(long double rhv) { return rhv * 1.e-9; }

	////Energy
	//MeV = 1.
	constexpr long double operator""_MeV(long double rhv) { return rhv; }
	//eV = * e-6 MeV
	constexpr long double operator""_eV(long double rhv) { return rhv * eV; }
	//keV = * e-3 MeV
	constexpr long double operator""_keV(long double rhv) { return rhv * keV; }
	//GeV = * e+3 MeV
	constexpr long double operator""_GeV(long double rhv) { return rhv * GeV; }
	//TeV = * e+6 MeV
	constexpr long double operator""_TeV(long double rhv) { return rhv * TeV; }

	////Angles
	//deg
	constexpr long double operator""_deg(long double rhv) { return rhv * deg; }
	//rad
	constexpr long double operator""_rad(long double rhv) { return rhv * rad; }

}

endChR

//the following using ain't the best, but it's good enough for this project
using namespace ChR::myLiterals;

////A time-benchmark class
#include <chrono>
#include <type_traits>

beginChR

template<typename D>
class TimeBench final {
	static_assert(std::chrono::__is_duration_v<D>, "You must use std::chrono::duration with the class ChR::TimeBench!");
	using theClock = std::chrono::time_point<std::chrono::high_resolution_clock>;
public:
	TimeBench(const char* functionName = "") : m_str(functionName) {
		m_start = std::chrono::high_resolution_clock::now();
	}
	~TimeBench() {
#if _HAS_CXX20
		D thePeriod = std::chrono::time_point_cast<D>(std::chrono::high_resolution_clock::now()).time_since_epoch()
			- std::chrono::time_point_cast<D>(m_start).time_since_epoch();
#else
		long long thePeriod = std::chrono::time_point_cast<D>(std::chrono::high_resolution_clock::now()).time_since_epoch().count()
			- std::chrono::time_point_cast<D>(m_start).time_since_epoch().count();
#endif // _HAS_CXX20
		if (m_str[0] == '\0')
			std::cout << "The time period of TimeBench was: " << thePeriod << '\n';
		else
			std::cout << "The time period of TimeBench in the \"" << m_str << "\" scope was: " << thePeriod << '\n';
	}
private:
	theClock m_start;
	const char* m_str;
};

endChR

#endif // !UnitsAndBench_hpp
