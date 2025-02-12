# Cherenkov project

- ChR = Cherenkov radiation
- Demonstrates how to use Cherenkov processes and which model to select;
- The aim is to obtain quasi-monochromatic Cherenkov spectral lines (ChR peaks further; experiment [^1]);
- The need for new Cherenkov processes is explained more thoroughly in [^2];
- The `ChR_app_exe` target is a standard Geant4 application;
- The `ChR_process_lib` target implements new Cherenkov models.

## ChR_app_exe

### ChR_project GEOMETRY DEFINITION

A world volume containing two additional volumes:

- ***Radiator***: The radiator is a thin plate (in experiment 200 um) inclined by a
specific angle so the emitted ChR could be extracted from the material (using a
perpendicular radiator and high-refractive-index material results in total internal
reflections). A material with a frequency-dependent refractive index must be used to
obtain ChR peaks.

- ***Detector***: The detector should have a small aperture (experimental: 35um in
radius) and be far from the radiator (experimental: 37.6cm). If all the previous
is satisfied, it is possible to observe ChR peaks instead of the typical whole spectrum
(only a small part of the emitted Cherenkov radiation is detected due to the minimal
solid angle of the detector).

### ChR_project PHYSICS LIST

All standard Geant4 physics modules are used. One can switch between various EM
physics sub-packages to test different effects (e.g., different MS models):
- ***emstandard_opt0*** - recommended standard EM physics for LHC
- ***emstandard_opt1*** - best CPU performance standard physics for LHC
- ***emstandard_opt2*** - similar fast simulation
- ***emstandard_opt3*** - best standard EM options - analog to "local" above
- ***emstandard_opt4*** - best current advanced EM options standard + low energy
- ***emstandardWVI***   - standard EM physics and WentzelVI multiple scattering
- ***emstandardSS***    - standard EM physics and single scattering model
- ***emstandardGS***    - standard EM physics and Goudsmit-Saunderson multiple scatt.
- ***emlivermore***     - low-energy EM physics using Livermore data
- ***empenelope***      - low-energy EM physics implementing Penelope models
- ***emlowenergy***     - low-energy EM physics implementing experimental
                    low-energy model

Also, one can choose between 3 optical physics:
- ***G4OpticalPhysics***      - A standard Cherenkov model typically used in Geant4
- ***G4OpticalPhysics_opt1*** - A standard Cherenkov model with advanced physics tables
                that can even consider exotic refractive indices (see below)
- ***G4OpticalPhysics_opt2*** - A Cherenkov process that can use various ChR models depending
                on logical volume (this one should be used in this example
                because we need a thin-target model - see below)

### ChR_project DEFSNCONSTANTS.HH

One can select different preprocessor definitions to change the execution mode:
1. `standardRun` - photons are emitted as they should be. Due to the geometry specifics
            that means a very low efficiency can be expected, and expected run times are
            very long (to obtain normal-intensity ChR peaks);
   1. `boostEfficiency` - instead of being emitted in all directions, the emitted photons
            are directed toward the detector. With this definition selected, one can obtain
            high-intensity ChR peaks very fast;
   2. `followMinMaxValues` - can be used to help determine limits for boostEfficiency;
2. `captureChRPhotonEnergyDistribution` - used to obtain spectra of ChR for exotic refractive
            indices (a test that shows expected results) - ChR peaks are not followed here!
3. If none of the above is selected, one can test the performance of various Cherenkov processes.

### ChR_project THE PRIMARY GENERATOR

Electrons of 855MeV are used in the experiment. One can also use ions (such an experiment is
planned in JINR). To select them, use standard UI `/gun/` commands.

One can also change the beam profile:
- default: pencil-like beams
- Gauss beam - experimental sigma = 536um
- include beam divergence (also not included by default)

## ChR_process_lib

The whole library target is written using the Geant4 namings.
Here, only some library features are provided, while one can find more information
through the comments in the code and article [^2]. Some more significant features:

- ***New optical physics***: To load new Cherenkov processes, it was also necessary to
rework the optical physics. Here, one can find two such classes, i.e.,
`G4OpticalPhysics_option1` and `G4OpticalPhysics_option2`. The former class loads all
processes in separate virtual methods, which means one does not need to rewrite the
whole optical physics each time it is necessary to change an optical process.
Instead, the former class can be inherited, and only specific methods (processes)
can be overridden. That is done in the latter optical physics, which loads another
new Cherenkov process.

- ***G4CherenkovProcess***: It is a wrapper class for various Cherenkov models. The goal
of this class is to select what Cherenkov model should be executed, which is done
with the help of the `G4ExtraOpticalParameters` class. The process is loaded through
`G4OpticalPhysics_option2`.

- ***Cherenkov models***: All Cherenkov models must inherit an abstract class
`G4BaseChR_Model`. This library provides two such models, i.e., `G4StandardChR_Model`
and `G4ThinTargetChR_Model`. The latter can be used to explain the experimental
data [^1]. The main improvement (over the G4Cerenkov class) of the former (standard)
model is that now it can consider any refractive index dependencies through new physics
tables (see more detailed explanation in [^2]).

- ***Accessing physics data***: To achieve the main improvement previously mentioned, it
is necessary to access stored vectors of the `G4PhysicsVector` class. Those
vectors are protected, and using `reinterpret_cast` can solve the problem. To
understand the source of the problem, see [^2].

- ***G4StandardCherenkovProcess***: The class is loaded through the
`G4OpticalPhysics_option1`, and the main idea of the class is to boost the code
performance. In general, the class works almost identically to `G4StandardChR_Model`,
while it can improve the performance for complex detectors (if there are many
logical volumes). This class should be used if the standard Cherenkov radiation
model is the only model utilized in simulations (for most users' needs). The main
difference compared to the `G4Cerenkov` class is in the new physics tables.

## How to use the project
To use the project, the user must have installed the Geant4 toolkit. If there is
Geant4, the project can be generated via the provided `CMakeLists.txt` file as usual.

Nevertheless, if one wants to implement the new Cherenkov processes, it is
necessary to change the executable target in CMake files, i.e., to change the
`ChR_app_exe` to some `user_exe` target.

## References

[^1]: A. Potylitsyn, G. Kube, A. Novokshonov, A. Vukolov, S. Gogolev, B. Alexeev,
P. Klag, W. Lauth, First observation of quasi-monochromatic optical Cherenkov
radiation in a dispersive medium (quartz), Phys. Lett. A 417 (2021) 127680;
DOI: [10.1016/j.physleta.2021.127680](https://www.sciencedirect.com/science/article/pii/S0375960121005442)

[^2]: B. Đurnić, A. Potylitsyn, A. Bogdanov, S. Gogolev, On the rework and
development of new Geant4 Cherenkov models, JINST 20 (2025) P02008;
DOI: [10.1088/1748-0221/20/02/P02008](https://doi.org/10.1088/1748-0221/20/02/P02008)