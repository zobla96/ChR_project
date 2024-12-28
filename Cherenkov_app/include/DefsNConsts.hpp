//##########################################
//#######         VERSION 0.5        #######
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

// Don't want to waste time, so I'm only partially implementing the definition of 'standardRun'.
// It won't compile into a great application but it will do the job... after all it's an extremely simple app.
#if 1
#define standardRun
// 1 -> standard ChR project with a thin target
// 0 -> only obtain the energy distribution of ChR information (to check exotic refractive indices)

#elif 1
#define captureChRPhotonEnergyDistribution
// 1 -> to obtain Cherenkov radiation energy distribution in a non-standard run
// 0 -> to measure performance of Cherenkov process in a non-standard run

#endif

beginChR

//=======ChR helper variables=======
inline thread_local std::random_device g_rd{};
inline thread_local std::mt19937 g_mtGen{ g_rd() };

endChR

#endif // !DefsNConsts_hpp