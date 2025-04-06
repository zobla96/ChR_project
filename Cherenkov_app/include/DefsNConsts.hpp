//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef DefsNConsts_hpp
#define DefsNConsts_hpp

//For the ChR_app_exe target this header should be included at the very beginning of each header
#define beginChR namespace ChR {
#define endChR }

//G4 header
#include "globals.hh"
//std:: headers
#include <fstream>
#include <random>

#if 1
#define standardRun
// 1 -> standard ChR project with a thin target
// 0 -> only obtain the energy distribution of ChR information (to check exotic refractive indices)
  #if 1
  #define boostEfficiency
// 1 -> to enable boostEfficiency - tempering with photon emission angles so they would not fly all around, but
//      towards the detector. If changing the geometry, one will also have a warning flag to prevent a possible error
// 0 -> no boosting efficiency - the phi angle of emitted Cherenkov photons is in the range [0, 2*pi)
  constexpr bool g_throwErrorForNonDefault = true;
    #if 0
    #define followMinMaxValues
    inline double g_maxPhiValue = -DBL_MAX;
    inline double g_minPhiValue = DBL_MAX;
    inline double g_maxThetaValue = -DBL_MAX;
    inline double g_minThetaValue = DBL_MAX;
    #endif
  #endif // boostEfficiency

#elif 0
#define captureChRPhotonEnergyDistribution
// 1 -> to obtain Cherenkov radiation energy distribution in a non-standard run
// 0 -> to measure performance of Cherenkov process in a non-standard run

#endif // standardRun

beginChR

//=======ChR helper variables=======
inline thread_local std::random_device g_rd{};
inline thread_local std::mt19937 g_mtGen{ g_rd() };

// I'm too lazy to go around obtaining instances normally, so I'm just gonna save them here
// (more like thread local singletons, but without using GetInstance methods)
class DetectorConstruction;
class RunAction;
class EventAction;
class PrimaryGeneratorAction;
class StackingAction;
class TrackingAction;
class SteppingAction;

inline DetectorConstruction* g_detectorConstruction = nullptr;
inline thread_local RunAction* g_runAction = nullptr;
inline thread_local PrimaryGeneratorAction* g_primaryGenerator = nullptr;
inline thread_local StackingAction* g_stackingAction = nullptr;
#ifdef standardRun
inline thread_local EventAction* g_eventAction = nullptr;
inline thread_local TrackingAction* g_trackingAction = nullptr;
inline thread_local SteppingAction* g_steppingAction = nullptr;
#endif // standardRun

endChR

#endif // !DefsNConsts_hpp