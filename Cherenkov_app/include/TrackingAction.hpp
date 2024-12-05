//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef TrackingAction_hpp
#define TrackingAction_hpp

//User built headers
#include "PrimaryGeneratorAction.hpp"
//G4 headers
#include "G4UserTrackingAction.hh"
#include "G4Track.hh"

beginChR

struct SpecificTrackData {
	SpecificTrackData() = default;
	~SpecificTrackData() = default;
	SpecificTrackData(const SpecificTrackData&) = default;
	SpecificTrackData& operator=(const SpecificTrackData&) = default;

	double m_totalE = 0.;
	double m_deltaE = 0.;
	double m_stepLength = 0.;
	double m_beta = 0.;
	G4ThreeVector m_globalDirection{};
#ifdef followMinMaxValues
	double m_phiValue = DBL_MAX;
	double m_thetaValue = DBL_MAX;
#endif // followMinMaxValues

};

class TrackingAction final : public G4UserTrackingAction {
public:
	TrackingAction() = default;
	~TrackingAction() override = default;
	void PreUserTrackingAction(const G4Track*) override;
	//void PostUserTrackingAction(const G4Track*) override;
	//=======Set inlines=======
	inline void SetTotalEnergy(const G4Track*, const double);
	inline void SetTotalEnergy(const int trackID, const double);
	inline void SetDeltaEnergy(const G4Track*, const double);
	inline void SetDeltaEnergy(const int trackID, const double);
	inline void SetStepLength(const G4Track*, const double);
	inline void SetStepLength(const int trackID, const double);
	inline void SetBeta(const G4Track*, const double);
	inline void SetBeta(const int trackID, const double);
	inline void SetGlobalDirection(const G4Track*, const G4ThreeVector&);
	inline void SetGlobalDirection(const int trackID, const G4ThreeVector&);
#ifdef followMinMaxValues
	inline void SetPhiValue(const G4Track*, const double);
	inline void SetPhiValue(const int trackID, const double);
	inline void SetThetaValue(const G4Track*, const double);
	inline void SetThetaValue(const int trackID, const double);
#endif // followMinMaxValues
	//=======Get inlines=======
	[[nodiscard]] inline double GetTotalEnergy(const G4Track*) const;
	[[nodiscard]] inline double GetTotalEnergy(const int trackID) const;
	[[nodiscard]] inline double GetDeltaEnergy(const G4Track*) const;
	[[nodiscard]] inline double GetDeltaEnergy(const int trackID) const;
	[[nodiscard]] inline double GetStepLength(const G4Track*) const;
	[[nodiscard]] inline double GetStepLength(const int trackID) const;
	[[nodiscard]] inline double GetBeta(const G4Track*) const;
	[[nodiscard]] inline double GetBeta(const int trackID) const;
	[[nodiscard]] inline const G4ThreeVector& GetGlobalDirection(const G4Track*) const;
	[[nodiscard]] inline const G4ThreeVector& GetGlobalDirection(const int trackID) const;
#ifdef followMinMaxValues
	[[nodiscard]] inline double GetPhiValue(const G4Track*) const;
	[[nodiscard]] inline double GetPhiValue(const int trackID) const;
	[[nodiscard]] inline double GetThetaValue(const G4Track*) const;
	[[nodiscard]] inline double GetThetaValue(const int trackID) const;
#endif // followMinMaxValues
	[[nodiscard]] inline SpecificTrackData& GetOrCreateSpecificTrackData(const G4Track*);
	[[nodiscard]] inline SpecificTrackData& GetOrCreateSpecificTrackData(const int trackID);
	// nullptr if there's no such data
	[[nodiscard]] inline SpecificTrackData* GetSpecificTrackData(const G4Track*);
	[[nodiscard]] inline SpecificTrackData* GetSpecificTrackData(const int trackID);
	//=======Inlines=======
	inline void AddToTotalEnergy(const G4Track*, const double);
	inline void AddToDeltaEnergy(const G4Track*, const double);
	inline void AddToStepLength(const G4Track*, const double);
private:
	//for this project, only deltaE and stepLng can be used... for dosimetry it would be nice to go with totalE as well.
	//Still, keeping it as std::tuple with totalE for some other projects, so I could just copy/paste in other projects
	std::map<int, SpecificTrackData> m_specificTrackDataMap;
};

//=======Set inlines=======
void TrackingAction::SetTotalEnergy(const G4Track* aTrack, const double value) {
	m_specificTrackDataMap.at(aTrack->GetTrackID()).m_totalE = value;
}
void TrackingAction::SetTotalEnergy(const int trackID, const double value) {
	m_specificTrackDataMap.at(trackID).m_totalE = value;
}
void TrackingAction::SetDeltaEnergy(const G4Track* aTrack, const double value) {
	m_specificTrackDataMap.at(aTrack->GetTrackID()).m_deltaE = value;
}
void TrackingAction::SetDeltaEnergy(const int trackID, const double value) {
	m_specificTrackDataMap.at(trackID).m_deltaE = value;
}
void TrackingAction::SetStepLength(const G4Track* aTrack, const double value) {
	m_specificTrackDataMap.at(aTrack->GetTrackID()).m_stepLength = value;
}
void TrackingAction::SetStepLength(const int trackID, const double value) {
	m_specificTrackDataMap.at(trackID).m_stepLength = value;
}
void TrackingAction::SetBeta(const G4Track* aTrack, const double value) {
	m_specificTrackDataMap.at(aTrack->GetTrackID()).m_beta = value;
}
void TrackingAction::SetBeta(const int trackID, const double value) {
	m_specificTrackDataMap.at(trackID).m_beta = value;
}
void TrackingAction::SetGlobalDirection(const G4Track* aTrack, const G4ThreeVector& value) {
	m_specificTrackDataMap.at(aTrack->GetTrackID()).m_globalDirection = value.unit();
}
void TrackingAction::SetGlobalDirection(const int trackID, const G4ThreeVector& value) {
	m_specificTrackDataMap.at(trackID).m_globalDirection = value.unit();
}

#ifdef followMinMaxValues
void TrackingAction::SetPhiValue(const G4Track* aTrack, const double value) {
	m_specificTrackDataMap.at(aTrack->GetTrackID()).m_phiValue = value;
}
void TrackingAction::SetPhiValue(const int trackID, const double value) {
	m_specificTrackDataMap.at(trackID).m_phiValue = value;
}
void TrackingAction::SetThetaValue(const G4Track* aTrack, const double value) {
	m_specificTrackDataMap.at(aTrack->GetTrackID()).m_thetaValue = value;
}
void TrackingAction::SetThetaValue(const int trackID, const double value) {
	m_specificTrackDataMap.at(trackID).m_thetaValue = value;
}
#endif // followMinMaxValues

//=======Get inlines=======
double TrackingAction::GetTotalEnergy(const G4Track* aTrack) const {
	return m_specificTrackDataMap.at(aTrack->GetTrackID()).m_totalE;
}
double TrackingAction::GetTotalEnergy(const int trackID) const {
	return m_specificTrackDataMap.at(trackID).m_totalE;
}
double TrackingAction::GetDeltaEnergy(const G4Track* aTrack) const {
	return m_specificTrackDataMap.at(aTrack->GetTrackID()).m_deltaE;
}
double TrackingAction::GetDeltaEnergy(const int trackID) const {
	return m_specificTrackDataMap.at(trackID).m_deltaE;
}
double TrackingAction::GetStepLength(const G4Track* aTrack) const {
	return m_specificTrackDataMap.at(aTrack->GetTrackID()).m_stepLength;
}
double TrackingAction::GetStepLength(const int trackID) const {
	return m_specificTrackDataMap.at(trackID).m_stepLength;
}
double TrackingAction::GetBeta(const G4Track* aTrack) const {
	return m_specificTrackDataMap.at(aTrack->GetTrackID()).m_beta;
}
double TrackingAction::GetBeta(const int trackID) const {
	return m_specificTrackDataMap.at(trackID).m_beta;
}
const G4ThreeVector& TrackingAction::GetGlobalDirection(const G4Track* aTrack) const {
	return m_specificTrackDataMap.at(aTrack->GetTrackID()).m_globalDirection;
}
const G4ThreeVector& TrackingAction::GetGlobalDirection(const int trackID) const {
	return m_specificTrackDataMap.at(trackID).m_globalDirection;
}

#ifdef followMinMaxValues
double TrackingAction::GetPhiValue(const G4Track* aTrack) const {
	return m_specificTrackDataMap.at(aTrack->GetTrackID()).m_phiValue;
}
double TrackingAction::GetPhiValue(const int trackID) const {
	return m_specificTrackDataMap.at(trackID).m_phiValue;
}
double TrackingAction::GetThetaValue(const G4Track* aTrack) const {
	return m_specificTrackDataMap.at(aTrack->GetTrackID()).m_thetaValue;
}
double TrackingAction::GetThetaValue(const int trackID) const {
	return m_specificTrackDataMap.at(trackID).m_thetaValue;
}
#endif // followMinMaxValues

SpecificTrackData& TrackingAction::GetOrCreateSpecificTrackData(const G4Track* aTrack) {
	return m_specificTrackDataMap[aTrack->GetTrackID()];
}
SpecificTrackData& TrackingAction::GetOrCreateSpecificTrackData(const int trackID) {
	return m_specificTrackDataMap[trackID];
}
SpecificTrackData* TrackingAction::GetSpecificTrackData(const G4Track* aTrack) {
	try {
		return &m_specificTrackDataMap.at(aTrack->GetTrackID());
	}
	catch (std::out_of_range) {
		return nullptr;
	}
}
SpecificTrackData* TrackingAction::GetSpecificTrackData(const int trackID) {
	try {
		return &m_specificTrackDataMap.at(trackID);
	}
	catch (std::out_of_range) {
		return nullptr;
	}
}
//=======Inlines=======
void TrackingAction::AddToTotalEnergy(const G4Track* aTrack, const double value) {
	m_specificTrackDataMap.at(aTrack->GetTrackID()).m_totalE += value;
}
void TrackingAction::AddToDeltaEnergy(const G4Track* aTrack, const double value) {
	m_specificTrackDataMap.at(aTrack->GetTrackID()).m_deltaE += value;
}
void TrackingAction::AddToStepLength(const G4Track* aTrack, const double value) {
	m_specificTrackDataMap.at(aTrack->GetTrackID()).m_stepLength += value;
}

endChR

#endif // !TrackingAction_hpp