//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "StackingAction.hpp"
//G4 headers
#include "G4AnalysisManager.hh"

beginChR

//=========public ChR::StackingAction:: methods=========

StackingAction::StackingAction()
#ifdef boostEfficiency
: m_rotationMatrix(G4PhysicalVolumeStore::GetInstance()->GetVolume("radiatorPhys")->GetRotation()->inverse()),
m_withGaussSigma(false), m_deltaPhi(180.), m_thetaMin(-DBL_MAX), m_thetaMax(DBL_MAX) {
	// taking inverse of the matrix as the G4RotationMatrix implements active transforms
	p_rindexVector = G4Material::GetMaterial(g_detectorConstruction->GetRadiatorMaterialName())->
		GetMaterialPropertiesTable()->GetProperty(kRINDEX);
	if (!dynamic_cast<const PhysicsList*>(G4RunManager::GetRunManager()->GetUserPhysicsList())->GetPhysics("OpticalPhysics_op2"))
		return;
	G4LogicalVolume* aLayerLogic = G4LogicalVolumeStore::GetInstance()->GetVolume("radiatorLayerLogic", false);
	if (aLayerLogic) {
		if (G4ExtraOpticalParameters::GetInstance()->FindChRMatData(aLayerLogic)->m_executeModel == 1)
			m_withGaussSigma = true;
	}
	else if (G4ExtraOpticalParameters::GetInstance()->FindChRMatData(
			G4LogicalVolumeStore::GetInstance()->GetVolume("radiatorLogic"))->m_executeModel == 1)
		m_withGaussSigma = true;
}
#else
{}
#endif // boostEfficiency

G4ClassificationOfNewTrack StackingAction::ClassifyNewTrack(const G4Track* aTrack) {
	if (aTrack->GetTrackID() <= g_primaryGenerator->GetNoOfParticles())
		return fUrgent;
#ifdef standardRun
  #ifdef boostEfficiency
	/*
	The following is just a simple and quick solution to significantly increase the efficiency of simulations.
	The idea of the experiment is to use a very small solid detector angle. That also implies that most of
	Cherenkov photons fly around without being detected! Because of that, the idea here is to direct all emitted
	photons towards the detector without violating Monte Carlo principles. Still, this can be done even better
	and one can even determine the expected angles, wavelengths, etc., in the run time (I might include it in
	the future - maybe even use some ray/path tracing algorithms).
	As this code cannot adapt the conditions in run time, one will get a fatal exception (it can and should
	be turned off to change the default geometry - it's done to make sure the user notices and understands
	the problem). Anyhow, all member variables of this class are set in the beginning of each run (in RunAction),
	and if the user wants (it should be done for non-default runs) to boost efficiency, those values should be
	changed. For now, this is done manually.
	In this simple approach, I analyze only angles, while even wavelength of photons play a critical role. Still,
	as written, maybe in the future.
	See the NOTE below.
	*/
	if (aTrack->GetParticleDefinition()->GetParticleName() != "opticalphoton")
		return fKill;
	G4ThreeVector& nonConstPhotonDirection = const_cast<G4ThreeVector&>(aTrack->GetMomentumDirection());
	G4ThreeVector& nonConstPolarization = const_cast<G4ThreeVector&>(aTrack->GetPolarization());
	const SpecificTrackData* parentData = g_trackingAction->GetSpecificTrackData(aTrack->GetParentID());
	double newPhi = (90 + (G4UniformRand() - 0.5) * m_deltaPhi) * deg;
	double sinNewPhi = std::sin(newPhi);
	double cosNewPhi = std::cos(newPhi);
	if (!m_withGaussSigma) {
		double cosTheta = parentData->m_globalDirection.dot(nonConstPhotonDirection);
		double sinTheta = std::sqrt((1. - cosTheta) * (1 + cosTheta));
		nonConstPhotonDirection.set(sinTheta * cosNewPhi, sinTheta * sinNewPhi, cosTheta);
		nonConstPhotonDirection.rotateUz(parentData->m_globalDirection);
		nonConstPolarization.set(cosTheta * cosNewPhi, cosTheta * sinNewPhi, -sinTheta);
		nonConstPolarization.rotateUz(parentData->m_globalDirection);
#ifdef followMinMaxValues
		SpecificTrackData& theData = g_trackingAction->GetOrCreateSpecificTrackData(aTrack);
		// to remove values from previous runs (if I'm not mistaken, G4TrackingAction is not created/destroyed all the time)
		theData = SpecificTrackData{};
		theData.m_phiValue = newPhi;
#endif // followMinMaxValues
		return fUrgent;
	}
	double sampledRI = p_rindexVector->Value(aTrack->GetTotalEnergy());
	double waveLng = 1.239841984e-6 * m * eV / aTrack->GetTotalEnergy();
	// for some reason I cannot find the direct way to obtain parent's track (e.g., through the id)!
	// that's why I saved some data in the tracking action. Note that the step can be used this way
	// because Cherenkov process sets the flag "track secondaries first"... that means that the will
	// not be removed before the secondary photons are processed! Otherwise, I would need to take a
	// more complex approach.
	double theAngleCos = parentData->m_globalDirection.dot(m_rotationMatrix(G4ThreeVector{ 0., 0., 1. }));
	if (theAngleCos == 0.)
		return fKill;
	double thetaChR = std::acos(1. / (sampledRI * parentData->m_beta));
	// NOTE
	// It is very important to keep in mind that this equation gives good results for angles ~90 deg.
	// If the detector's solid angle is increased, it's very possible that one will need to change to
	// the more general approach, similar to the one in G4ThinTargetChR_Model
	double gaussSigma = 0.42466 * waveLng * theAngleCos / (2 * sampledRI * g_detectorConstruction->GetRadiatorThickness() *
		(std::tan(thetaChR - g_detectorConstruction->GetRadiatorAngle()) +
			std::tan(g_detectorConstruction->GetRadiatorAngle())));

	thetaChR = G4RandGauss::shoot(thetaChR, gaussSigma);
	if (thetaChR > m_thetaMax || thetaChR < m_thetaMin) // some 20-25% boost in time
		return fKill;
	double cosTheta = std::cos(thetaChR);
	double sinTheta = std::sin(thetaChR);
	nonConstPhotonDirection.set(sinTheta * cosNewPhi, sinTheta * sinNewPhi, cosTheta);
	nonConstPhotonDirection.rotateUz(parentData->m_globalDirection);
	nonConstPolarization.set(cosTheta * cosNewPhi, cosTheta * sinNewPhi, -sinTheta);
	nonConstPolarization.rotateUz(parentData->m_globalDirection);
#ifdef followMinMaxValues
	SpecificTrackData& theData = g_trackingAction->GetOrCreateSpecificTrackData(aTrack);
	// to remove values from previous runs (if I'm not mistaken, G4TrackingAction is not created/destroyed all the time)
	theData = SpecificTrackData{};
	theData.m_phiValue = newPhi;
	theData.m_thetaValue = thetaChR;
#endif // followMinMaxValues
	return fUrgent;

  #else // version 0.5 and earlier
	if (aTrack->GetParticleDefinition()->GetParticleName() == "opticalphoton") {
		if (aTrack->GetMomentumDirection().y() <= 0)
			//Those wouldn't be detected while they would increase the computation time significantly
			//Due to high refractive index, they would experience total internal reflections
			return fKill;
		else
			return fUrgent;
	}
	//While the following return ain't perfect, additional particles could only raise the background.
	//Due to the geometry, the raise in peaks' intensity would be insignificant (I believe).
	//Yet, those might increase the already long computation times
	return fKill;
  #endif // boostEfficiency
#else
  #ifdef captureChRPhotonEnergyDistribution
	if (aTrack->GetParticleDefinition()->GetParticleName() == "opticalphoton") {
		/*
		for checking performance, undef "captureChRPhotonEnergyDistribution".
		It significantly slows down a multi-thread app, as if some
		atomics are a problem (haven't analyzed). The previous "as if" is
		because I got better performance on 4 threads compared to 20
		which can be related to atomics' cache memory effects!
		*/
		G4double energy = aTrack->GetTotalEnergy();
		G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
		analysisManager->FillNtupleIColumn(0, 0);
		analysisManager->FillNtupleDColumn(1, energy / eV);
		analysisManager->FillNtupleDColumn(2, 1.239841984e-6 * m * eV / (energy * nm));
		analysisManager->FillNtupleDColumn(3, 0.);
		analysisManager->FillNtupleDColumn(4, 0.);
		analysisManager->AddNtupleRow();
	}
  #endif // captureChRPhotonEnergyDistribution
	return fKill;
#endif // standardRun
}

endChR