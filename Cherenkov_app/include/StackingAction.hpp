//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef StackingAction_hpp
#define StackingAction_hpp

//User built headers
#include "DefsNConsts.hpp"
#include "PrimaryGeneratorAction.hpp"
#include "DetectorConstruction.hpp"
#include "TrackingAction.hpp"
#include "PhysicsList.hpp"
//G4 headers
#include "G4UserStackingAction.hh"
#include "G4Track.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"

beginChR

class StackingAction final : public G4UserStackingAction {
public:
	StackingAction();
	~StackingAction() override = default;
	G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack) override;
#ifdef boostEfficiency
	// Set inlines
	inline void SetDeltaPhi(const double);
	inline void SetThetaMin(const double);
	inline void SetThetaMax(const double);
	// Get inlines
	[[nodiscard]] inline double GetDeltaPhi() const;
	[[nodiscard]] inline double GetThetaMin() const;
	[[nodiscard]] inline double GetThetaMax() const;
private:
	const G4RotationMatrix m_rotationMatrix; // local to global
	const G4PhysicsFreeVector* p_rindexVector = nullptr;
	bool m_withGaussSigma;
	double m_deltaPhi;
	double m_thetaMin;
	double m_thetaMax;
#endif // boostEfficiency
};

#ifdef boostEfficiency
// Set inlines
void StackingAction::SetDeltaPhi(const double value) {
	m_deltaPhi = value;
}
void StackingAction::SetThetaMin(const double value) {
	m_thetaMin = value;
}
void StackingAction::SetThetaMax(const double value) {
	m_thetaMax = value;
}
// Get inlines
double StackingAction::GetDeltaPhi() const {
	return m_deltaPhi;
}
double StackingAction::GetThetaMin() const {
	return m_thetaMin;
}
double StackingAction::GetThetaMax() const {
	return m_thetaMax;
}
#endif // boostEfficiency

endChR

#endif // !StackingAction_hpp