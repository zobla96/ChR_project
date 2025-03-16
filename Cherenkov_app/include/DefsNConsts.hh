//##########################################
//#######        VERSION 1.0.1       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef DefsNConsts_hh
#define DefsNConsts_hh

// For the ChR_app_exe target this header should be included at the very beginning of each header
#define beginChR namespace ChR {
#define endChR }

// G4 header
#include "globals.hh"
// std:: headers
#include <fstream>
#include <random>

#if 1
#define standardRun
// 1 -> standard ChR project with a thin target
// 0 -> use only the target for producing and analyzing Cherenkov radiation
  #define outputFolderBegin "standardRun - "
  #if 1
  #define boostEfficiency
  #undef outputFolderBegin
  #define outputFolderBegin "boostEfficiency - "
// 1 -> to enable boostEfficiency - tempering with photon emission angles so they would not fly all around, but
//      towards the detector (due to its very small aperture). If changing the geometry, one will also have a
//		warning flag to prevent a possible error
// 0 -> no boosting efficiency - the phi angle of emitted Cherenkov photons is in the range [0, 2*pi)
  constexpr G4bool g_throwErrorForNonDefault = true;
    #if 0
// 1 -> additionally follow the min and max values of phi and theta angles to determine the boostEfficiency limits easier
// 0 -> no need to follow the min and max values...
    #define followMinMaxValues
    inline G4double g_maxPhiValue = -DBL_MAX;
    inline G4double g_minPhiValue = DBL_MAX;
    inline G4double g_maxThetaValue = -DBL_MAX;
    inline G4double g_minThetaValue = DBL_MAX;
    #endif
  #endif // boostEfficiency

#elif 0
#define captureChRPhotonEnergyDistribution
#define outputFolderBegin "ChRPhotonEnergy - "
// 1 -> to obtain Cherenkov radiation energy distribution in a non-standard run
// 0 -> to measure performance of Cherenkov processes

#endif // standardRun

beginChR

//=======ChR helper variables=======
inline thread_local std::random_device g_rd{};
inline thread_local std::mt19937 g_mtGen{ g_rd() };

// One may obtain instances normally (e.g., using G4RunManager), but this is much
// simpler (like thread local singletons, but without using GetInstance methods)
class DetectorConstruction;
class RunAction;
class PrimaryGeneratorAction;
class StackingAction;
class TrackingAction;
class SteppingAction;

inline DetectorConstruction* g_detectorConstruction = nullptr;
inline thread_local RunAction* g_runAction = nullptr;
inline thread_local PrimaryGeneratorAction* g_primaryGenerator = nullptr;
inline thread_local StackingAction* g_stackingAction = nullptr;
#ifdef standardRun
inline thread_local TrackingAction* g_trackingAction = nullptr;
inline thread_local SteppingAction* g_steppingAction = nullptr;
#endif // standardRun

template<typename T>
constexpr bool is_Duration_v = false;
template<typename T, typename Period>
constexpr bool is_Duration_v<std::chrono::duration<T, Period>> = true;

template<typename D>
class TimeBench final {
	// ChR:: just to emphasize the origin of is_Duration_v
	static_assert(ChR::is_Duration_v<D>, "You must use std::chrono::duration with the class TimeBench!");
	using theClock = std::chrono::time_point<std::chrono::high_resolution_clock>;
public:
	TimeBench(const char* timerName = "")
		: m_name(timerName), m_timerStart(std::chrono::high_resolution_clock::now()) {}
	TimeBench(const std::string& timerName)
		: m_name(timerName), m_timerStart(std::chrono::high_resolution_clock::now()) {}
	TimeBench(const G4String& timerName)
		: m_name(timerName), m_timerStart(std::chrono::high_resolution_clock::now()) {}
	~TimeBench() {
		timerTime(m_name);
	}
	inline void reset() {
		m_timerStart = std::chrono::high_resolution_clock::now();
	}
	inline void timerTime(const G4String& nameString) const {
		G4String aReport{ "The time period " };
#if _HAS_CXX20
		D thePeriod = std::chrono::time_point_cast<D>(std::chrono::high_resolution_clock::now()).time_since_epoch()
			- std::chrono::time_point_cast<D>(m_timerStart).time_since_epoch();
#else
		long long thePeriod = std::chrono::time_point_cast<D>(std::chrono::high_resolution_clock::now()).time_since_epoch().count()
			- std::chrono::time_point_cast<D>(m_timerStart).time_since_epoch().count();
#endif
		std::stringstream aStr{};
		aStr << thePeriod;
		if (nameString.empty())
			aReport += G4String{ "was: " } + aStr.str().c_str() + "\n";
		else
			aReport += G4String{ "of \"" } + nameString + "\" was: " + aStr.str().c_str() + "\n";
		std::cout << aReport;
	}
	inline void SetName(const char* aName) {
		m_name = aName;
	}
	inline void SetName(const std::string& aName) {
		m_name = aName;
	}
	inline void SetName(const G4String& aName) {
		m_name = aName;
	}
	inline const G4String& GetName() const {
		return m_name;
	}
private:
	theClock m_timerStart;
	G4String m_name;
};

endChR

#endif // !DefsNConsts_hh