//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef EventAction_hpp
#define EventAction_hpp

//User built headers
#include "DetectorConstruction.hpp"
//G4 headers
#include "G4UserEventAction.hh"
//std:: headers
#include <vector>

beginChR

class EventAction final : public G4UserEventAction {
public:
	struct LayerData {
		LayerData() = default;
		~LayerData() = default;
		size_t m_count = 0;
		double m_lostE = 0.;
		double m_stepLng = 0.;
		LayerData& operator+=(const LayerData& other) {
			this->m_count += other.m_count;
			this->m_lostE += other.m_lostE;
			this->m_stepLng += other.m_stepLng;
			return *this;
		}
	};
	EventAction();
	~EventAction() override = default;
	//void BeginOfEventAction(const G4Event* anEvent) override;
	//void EndOfEventAction(const G4Event* anEvent) override;
	void AddToLayerDataVec(const size_t layerNo, const double lostE, const double stepLNG);
	//=======Get inlines=======
	inline const std::vector<LayerData>& GetLayerDataVec() const;
private:
	std::vector<LayerData> m_layerDataVec;
};

//=======Get inlines=======
const std::vector<EventAction::LayerData>& EventAction::GetLayerDataVec() const {
	return m_layerDataVec;
}

endChR

#endif // !EventAction_hpp