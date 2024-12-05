//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef SomeGlobalNamespace_hh
#define SomeGlobalNamespace_hh

#include "globals.hh"
#include "stdexcept"

class G4VPhysicalVolume;
class G4AffineTransform;

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
struct no_mother_physical_volume : std::logic_error {
	using BaseType = std::logic_error;

	no_mother_physical_volume()
		: BaseType("Mother's logical volume not found in registered physical volumes") {};
	no_mother_physical_volume(const G4String& aMsg)
		: BaseType(aMsg.c_str()) {}
	no_mother_physical_volume(const char* aMsg)
		: BaseType(aMsg) {}
	~no_mother_physical_volume() = default;
};
// I don't know if there's such a function in Geant4, so gonna write my own
// Returns the local->global transformation for a specific G4VPhysicalVolume
// The function cannot consider replicas inside other replicas (after all, I've
// never used it, so I'm not even sure if Geant4 allows it)
// This is needed because G4VPhysicalVolume does not provide GetMotherPhysical,
// but only GetMotherLogical
// To be used before having a touchable, like when building a physics
// It's assumed there's no overlapping
[[nodiscard]] G4AffineTransform GetLocalToGlobalTransformOfPhysicalVolume(const G4VPhysicalVolume*);
[[nodiscard]] G4AffineTransform GetGlobalToLocalTransformOfPhysicalVolume(const G4VPhysicalVolume*);
// The previous functions consider passive transforms and may not be used as active transforms!
// Still the same principle can be used for active transforms [one would just need to change from
// nextTransform *= GetLocalToGlobalTransformOfPhysicalVolume(nextPhys) into
// nextTransform = GetLocalToGlobalTransformOfPhysicalVolume(nextPhys) * nextTransform]...
// however, the problem here is that G4AffineTransform only supports passive transforms!
// That means, e.g., ApplyAxisTransform assumes <vec3| * M1^-1 * M2^-1 etc.,
// while it cannot consider the good order for active transforms, i.e., M2 * M1 * |vec3>
// won't work (the vec3 is still considered as <vec3|, instead of |vec3>).
// Still, I believe that should be solvable by including operator* between G4AffineTransform
// and G4ThreeVector. That way, one can use different ordering of the two.

#endif // !SomeGlobalNamespace_hh