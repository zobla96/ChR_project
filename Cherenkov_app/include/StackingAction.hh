//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef StackingAction_hh
#define StackingAction_hh

// user headers
#include "DefsNConsts.hh"
// G4 headers
#include "G4UserStackingAction.hh"
#include "G4AffineTransform.hh"

class G4Track;
class G4PhysicsFreeVector;

beginChR

class StackingAction final : public G4UserStackingAction {
public:
	StackingAction();
	~StackingAction() override = default;
	G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack) override;
#ifdef boostEfficiency
	// Set inlines
	inline void SetDeltaPhi(const G4double);
	inline void SetThetaMin(const G4double);
	inline void SetThetaMax(const G4double);
	// Get inlines
	[[nodiscard]] inline G4double GetDeltaPhi() const;
	[[nodiscard]] inline G4double GetThetaMin() const;
	[[nodiscard]] inline G4double GetThetaMax() const;
private:
	const G4RotationMatrix m_rotationMatrix; // local to global
	const G4PhysicsFreeVector* p_rindexVector = nullptr;
	G4double m_deltaPhi;
	G4double m_thetaMin;
	G4double m_thetaMax;
	G4bool m_withGaussSigma;
#endif // boostEfficiency
};

#ifdef boostEfficiency
// Set inlines
void StackingAction::SetDeltaPhi(const G4double value) {
	m_deltaPhi = value;
}
void StackingAction::SetThetaMin(const G4double value) {
	m_thetaMin = value;
}
void StackingAction::SetThetaMax(const G4double value) {
	m_thetaMax = value;
}
// Get inlines
G4double StackingAction::GetDeltaPhi() const {
	return m_deltaPhi;
}
G4double StackingAction::GetThetaMin() const {
	return m_thetaMin;
}
G4double StackingAction::GetThetaMax() const {
	return m_thetaMax;
}
#endif // boostEfficiency

endChR

#endif // !StackingAction_hh