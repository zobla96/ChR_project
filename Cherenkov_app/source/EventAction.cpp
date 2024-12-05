//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#include "EventAction.hpp"

beginChR

//=========public ChR::EventAction:: methods=========

EventAction::EventAction() {
	m_layerDataVec.reserve(g_detectorConstruction->GetNoOfRadLayers());
	for (size_t i = 0; i < m_layerDataVec.capacity(); i++)
		m_layerDataVec.emplace_back();
}

void EventAction::AddToLayerDataVec(const size_t layerNo, const double lostE, const double stepLNG) {
	LayerData& data = m_layerDataVec[layerNo];
	data.m_count++;
	data.m_lostE += lostE;
	data.m_stepLng += stepLNG;
}

endChR