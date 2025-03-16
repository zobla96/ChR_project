//##########################################
//#######        VERSION 1.0.1       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//G4 headers
#include "GlobalFunctions.hh"
#include "G4AffineTransform.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolume.hh"

// Affine matrix is used like a homogenous in CGI, just without the projection part for the rasterization
G4AffineTransform GetLocalToGlobalTransformOfPhysicalVolume(const G4VPhysicalVolume* aPhysicalVolume) {
	G4AffineTransform aFinalTransform;
	aFinalTransform = { aPhysicalVolume->GetRotation(), aPhysicalVolume->GetTranslation() };
	const G4PhysicalVolumeStore* theStore = G4PhysicalVolumeStore::GetInstance();
	const G4LogicalVolume* aMotherLogic = aPhysicalVolume->GetMotherLogical();
	if (aMotherLogic == nullptr)
		return aFinalTransform;
	// find the mother volume - using brute force
	for (const G4VPhysicalVolume* nextPhys : *theStore) {
		if (nextPhys->GetLogicalVolume() != aMotherLogic)
			continue;
		// if the logical volumes are the same, make sure that's really the mother,
		// i.e., that it's not just one of the replicas
		G4AffineTransform nextTransform{ aFinalTransform };
		nextTransform *= GetLocalToGlobalTransformOfPhysicalVolume(nextPhys);
		/*
		Originally, when I started writing this function, the idea was to check if the
		mother volume contains a point that's in the 'aPhysicalVolume'... however, nothing
		comes to mind on how to do that. Thus, I'm just returning the transform, meaning
		this does not work if there are replica volumes in replica volumes (if the mother
		is a replica).
		I need at least a global point or something... or is it possible that each of the
		mother volumes (outer replicas with a specific logic) must contain all of the inner
		replicas?? That would be strange?
		If that's the case, this function should be slightly modified and there would be no way
		to return a single G4AffineTransform
		*/
		return nextTransform;
	}
	// if we are here, we did not find a mother volume so we throw
	throw no_mother_physical_volume{};
}

G4AffineTransform GetGlobalToLocalTransformOfPhysicalVolume(const G4VPhysicalVolume* aPhysicalVolume) {
	return GetLocalToGlobalTransformOfPhysicalVolume(aPhysicalVolume).Inverse();
}