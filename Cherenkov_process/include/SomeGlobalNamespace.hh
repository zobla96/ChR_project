//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef SomeGlobalNamespace_hh
#define SomeGlobalNamespace_hh

#include "globals.hh"

//NOTE: I'm not sure if there's a Geant4 namespace used for global functions and similar... this should be yet changed
//find xValue or yValue of (xValue, yValue), as a liner fit from (x0, y0), (x1, y1)
[[nodiscard]] inline G4double G4LinearInterpolate2D_GetX(const G4double y0, const G4double y1, const G4double x0, const G4double x1, const G4double yValue) {
	if (x0 == x1) {
		const char* location = "Header SomeGlobalNamespace.hh; Function G4LinearInterpolate2D_GetX\n";
		const char* error = "x0 and x1 must not be the same!\n";
		G4Exception(location, "FE_SomeGlobNS01", FatalException, error);
	}
	G4double slope = (y0 - y1) / (x0 - x1);
	G4double xValue = x0 - (y0 - yValue) / slope;
	return xValue;
}

[[nodiscard]] inline G4double G4LinearInterpolate2D_GetY(const G4double y0, const G4double y1, const G4double x0, const G4double x1, const G4double xValue) {
	if (x0 == x1) {
		const char* location = "Header SomeGlobalNamespace.hh; Function G4LinearInterpolate2D_GetY\n";
		const char* error = "x0 and x1 must not be the same!\n";
		G4Exception(location, "FE_SomeGlobNS02", FatalException, error);
	}
	G4double slope = (y0 - y1) / (x0 - x1);
	G4double yValue = y0 - slope * (x0 - xValue);
	return yValue;
}

#endif // !SomeGlobalNamespace_hh