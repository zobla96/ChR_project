//##########################################
//#######        VERSION 1.1.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

/*
ABOUT THE HEADER
----------------

Prepare and check all the data before the run starts, and after
the run ends, move the obtained data into a separate folder and
generate a ReadMe files that describes what was used in the run.
*/

#pragma once
#ifndef RunAction_hh
#define RunAction_hh

// user headers
#include "DefsNConsts.hh"
// G4 headers
#include "G4UserRunAction.hh"

beginChR

class RunAction final : public G4UserRunAction
{
public:
  RunAction();
  ~RunAction() override = default;
  void BeginOfRunAction(const G4Run*) override;
  void EndOfRunAction(const G4Run*) override;
private:
  void PrepareRunData();
};

endChR

#endif // !RunAction_hh