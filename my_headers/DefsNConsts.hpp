#pragma once
#ifndef DefsNConsts_hpp
#define DefsNConsts_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//For the ChR_project this header should be included at the very beginning
#define beginChR namespace ChR {
#define endChR }

#include "globals.hh"

#include <fstream>
#include <random>

//according to compiler explorer, gcc doesn't have the following macro
#ifdef __GNUC__
#ifdef _HAS_NODISCARD
#define _NODISCARD [[nodiscard]]
#else
#define _NODISCARD
#endif // _HAS_CXX17
#endif // __GNUC__


beginChR

//=======A ChR helper function=======
_NODISCARD inline double LinearInterpolate2D(const double y0, const double y1, const double x0, const double x1, const double xValue, const double yValue) { //using copies
	if (x0 == x1) {
		const char* location = "Header DefsNConsts.hpp; Function ChR::LinearInterpolate2D(...)\n";
		const char* error = "x0 and x1 must not be the same!\n";
		G4Exception(location, "FE1003", FatalException, error);
	}
	double slope = (y0 - y1) / (x0 - x1);
	if (xValue != 0. && yValue != 0. || xValue == 0. && yValue == 0.) {
		const char* location = "Header DefsNConsts.hpp; Function ChR::LinearInterpolate2D(...)\n";
		const char* error = "One and only one of xValue and yValue must be 0!\n";
		G4Exception(location, "FE1004", FatalException, error);
		return -1.; //for compiler
	}
	else if (xValue != 0. /*&& yValue == 0.*/) {
		double val = y0 - slope * (x0 - xValue);
		return val;
	}
	else /*(xValue == 0. && yValue != 0.)*/
		return x0 - (y0 - yValue) / slope;
}
inline thread_local std::random_device g_rd{};
inline thread_local std::mt19937 g_mtGen{ g_rd() };
inline thread_local std::uniform_real_distribution g_uniformDist(0., 1.); //according to G4 used G4UniformRand
inline std::mutex g_lck;
inline std::ofstream g_oFS{ "LogOfRun.txt", std::ios::trunc | std::ios::out }; //for now not used

endChR

#endif // !DefsNConsts_hpp