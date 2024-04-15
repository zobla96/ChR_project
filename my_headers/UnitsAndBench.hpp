#pragma once
#ifndef UnitsAndBench_hpp
#define UnitsAndBench_hpp

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
#include "G4SystemOfUnits.hh"
//std:: headers
#include <iostream>

beginChR

namespace myLiterals {
	////Note: to use this, one must use floating_point, I didn't overload for ints (or anything else)
	////Length
	//millimeter = 1.
	constexpr double operator""_millimeter(long double rhv) { return rhv; }
	constexpr double operator""_millimeter2(long double rhv) { return rhv; }
	constexpr double operator""_millimeter3(long double rhv) { return rhv; }
	constexpr double operator""_mm(long double rhv) { return rhv; }
	constexpr double operator""_mm2(long double rhv) { return rhv; }
	constexpr double operator""_mm3(long double rhv) { return rhv; }
	//centi = * 10. mm
	constexpr double operator""_centimeter(long double rhv) { return rhv * cm; }
	constexpr double operator""_centimeter2(long double rhv) { return rhv * cm2; }
	constexpr double operator""_centimeter3(long double rhv) { return rhv * cm3; }
	constexpr double operator""_cm(long double rhv) { return rhv * cm; }
	constexpr double operator""_cm2(long double rhv) { return rhv * cm2; }
	constexpr double operator""_cm3(long double rhv) { return rhv * cm3; }
	//deci = * 10. mm
	constexpr double operator""_decimeter(long double rhv) { return rhv * 1.e2; }
	constexpr double operator""_decimeter2(long double rhv) { return rhv * 1.e4; }
	constexpr double operator""_decimeter3(long double rhv) { return rhv * 1.e6; }
	constexpr double operator""_dm(long double rhv) { return rhv * 1.e2; }
	constexpr double operator""_dm2(long double rhv) { return rhv * 1.e4; }
	constexpr double operator""_dm3(long double rhv) { return rhv * 1.e6; }
	//Si unit = * 100. mm
	constexpr double operator""_meter(long double rhv) { return rhv * m; }
	constexpr double operator""_meter2(long double rhv) { return rhv * m2; }
	constexpr double operator""_meter3(long double rhv) { return rhv * m3; }
	constexpr double operator""_m(long double rhv) { return rhv * m; }
	constexpr double operator""_m2(long double rhv) { return rhv * m2; }
	constexpr double operator""_m3(long double rhv) { return rhv * m3; }
	//kilo = * e+6 mm
	constexpr double operator""_kilometer(long double rhv) { return rhv * km; }
	constexpr double operator""_kilometer2(long double rhv) { return rhv * km2; }
	constexpr double operator""_kilometer3(long double rhv) { return rhv * km3; }
	constexpr double operator""_km(long double rhv) { return rhv * km; }
	constexpr double operator""_km2(long double rhv) { return rhv * km2; }
	constexpr double operator""_km3(long double rhv) { return rhv * km3; }
	//micro = * e-3 mm
	constexpr double operator""_micrometer(long double rhv) { return rhv * um; }
	constexpr double operator""_um(long double rhv) { return rhv * um; }
	//nano = * e-6 mm
	constexpr double operator""_nanometer(long double rhv) { return rhv * nm; }
	constexpr double operator""_nm(long double rhv) { return rhv * nm; }
	//pico = * e-9 mm
	constexpr double operator""_picometer(long double rhv) { return rhv * 1.e-9; }
	constexpr double operator""_pm(long double rhv) { return rhv * 1.e-9; }

	////Energy
	//MeV = 1.
	constexpr double operator""_MeV(long double rhv) { return rhv; }
	//eV = * e-6 MeV
	constexpr double operator""_eV(long double rhv) { return rhv * eV; }
	//keV = * e-3 MeV
	constexpr double operator""_keV(long double rhv) { return rhv * keV; }
	//GeV = * e+3 MeV
	constexpr double operator""_GeV(long double rhv) { return rhv * GeV; }
	//TeV = * e+6 MeV
	constexpr double operator""_TeV(long double rhv) { return rhv * TeV; }

	////Angles
	//deg
	constexpr double operator""_deg(long double rhv) { return rhv * deg; }
	//rad
	constexpr double operator""_rad(long double rhv) { return rhv * rad; }

}

endChR

using namespace ChR::myLiterals;

////A time-benchmark class
#include <chrono>
#include <type_traits>

beginChR


template<typename D>
class TimeBench final {
	static_assert(std::chrono::_Is_duration_v<D>, "You must use std::chrono::duration with the class ChR::TimeBench!");
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
		size_t thePeriod = std::chrono::time_point_cast<D>(std::chrono::high_resolution_clock::now()).time_since_epoch().count()
			- std::chrono::time_point_cast<D>(m_start).time_since_epoch().count();
#endif // _HAS_CXX20
		if (m_str && m_str[0] == '\0')
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