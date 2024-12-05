# Version 0.6

***ChR_process_lib***:

-*G4ThinTargetChR_Model* - the model has been reworked to directly include
the inclination of radiators instead of using approximations.

-*G4CherenkovMatData* - major changes in struct member variables according
to the new `G4ThinTargetChR_Model`. Now one does not need to specify the
thickness of the material manually, but one still needs to select the
`G4ThinTargetChR_Model` class for execution manually. Also, most member
variables are set as private so only friend classes can change them.

-*SomeGlobalNamespace* - added functions to obtain local->global and
global->local passive transformations before the tracking starts.

-*G4ChRPhysicsTableData* - added a missing `delete p_bigBetaCDFVector`
in the move assignment operator, i.e., a memory leak fixed. In general,
it is intended to be used with nullptr, but it's still more correct
to use the `delete` keyword.

-Some additional style changes that do not impact the code

***ChR_app_exe***:

-*DefsNConstants.hpp* - added a possibility to boost efficiency. Previously,
photons were emitted in 2*Pi angles while the solid angle of the detector was
minimal. Thus, one needed a lot of time for a single run to obtain decent
statistics. With the boosted efficiency, photons are emitted towards the
detector, and one can quickly obtain high-intensity spectral lines.
Besides boosting efficiency, global pointers for essential objects have been
added for easier access.

-*PrimaryGeneratorAction* - the class is no longer a singleton after adding
the global pointers.

-*DetectorConstruction* - the class is no longer a singleton after adding
the global pointers.

-*ActionInitialization* - definition of most of the global pointers.

-*ProcessCsvData<Args...>* - validation of parameter packs changed from run-time
to compile-time.

-*RunAction* - adapted specialization for `ProcessCsvData<...>::ReadMePrintAboutCurrentProjectData(...)`.
Also, the methods are adapted to prepare and check the data for the `boostEfficiency`
runs.

-*StackingAction* - The class is now responsible for directing the emitted photons
towards the detector (when `boostEfficiency` is defined).

-*TrackingAction* - new track data can be stored according to the needs to
boost run efficiency.

-*SteppingAction* - adapted to obtain the data to boost the efficiency.
Also, atomic variables changed from the default memory order (`seq_cst`) to
the relaxed one.

-Some additional style changes that do not impact the code