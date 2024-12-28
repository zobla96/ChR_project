//##########################################
//#######         VERSION 0.5        #######
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

class TrackingAction final : public G4UserTrackingAction {
public:
	TrackingAction();
	~TrackingAction() override = default;
	void PreUserTrackingAction(const G4Track*) override;
	void PostUserTrackingAction(const G4Track*) override;
	//=======Set inlines=======
	inline void SetLayerTotalEnergy(const G4Track*, const double);
	inline void SetLayerDeltaEnergy(const G4Track*, const double);
	inline void SetLayerStep(const G4Track*, const double);
	//=======Get inlines=======
	[[nodiscard]] inline double GetLayerTotalEnergy(const G4Track*) const;
	[[nodiscard]] inline double GetLayerDeltaEnergy(const G4Track*) const;
	[[nodiscard]] inline double GetLayerStep(const G4Track*) const;
	//=======Inlines=======
	inline void AddToLayerTotalEnergy(const G4Track*, const double);
	inline void AddToLayerDeltaEnergy(const G4Track*, const double);
	inline void AddToLayerStep(const G4Track*, const double);
private:
	const unsigned int& r_noOfPrimaries;
	//for this project, only deltaE and stepLng can be used... for dosimetry it would be nice to go with totalE as well.
	//Still, keeping it as std::tuple with totalE for some other projects, so I could just copy/paste in other projects
	std::map<int, std::tuple<double /*totalE*/, double /*deltaE*/, double /*stepLng*/>> m_layerData;
};

//=======Set inlines=======
void TrackingAction::SetLayerTotalEnergy(const G4Track* aTrack, const double value) {
	std::get<0>(m_layerData.at(aTrack->GetTrackID())) = value;
}
void TrackingAction::SetLayerDeltaEnergy(const G4Track* aTrack, const double value) {
	std::get<1>(m_layerData.at(aTrack->GetTrackID())) = value;
}
void TrackingAction::SetLayerStep(const G4Track* aTrack, const double value) {
	std::get<2>(m_layerData.at(aTrack->GetTrackID())) = value;
}
//=======Get inlines=======
double TrackingAction::GetLayerTotalEnergy(const G4Track* aTrack) const {
	return std::get<0>(m_layerData.at(aTrack->GetTrackID()));
}
double TrackingAction::GetLayerDeltaEnergy(const G4Track* aTrack) const {
	return std::get<1>(m_layerData.at(aTrack->GetTrackID()));
}
double TrackingAction::GetLayerStep(const G4Track* aTrack) const {
	return std::get<2>(m_layerData.at(aTrack->GetTrackID()));
}
//=======Inlines=======
void TrackingAction::AddToLayerTotalEnergy(const G4Track* aTrack, const double value) {
	std::get<0>(m_layerData.at(aTrack->GetTrackID())) += value;
}
void TrackingAction::AddToLayerDeltaEnergy(const G4Track* aTrack, const double value) {
	std::get<1>(m_layerData.at(aTrack->GetTrackID())) += value;
}
void TrackingAction::AddToLayerStep(const G4Track* aTrack, const double value) {
	std::get<2>(m_layerData.at(aTrack->GetTrackID())) += value;
}

endChR

#endif // !TrackingAction_hpp