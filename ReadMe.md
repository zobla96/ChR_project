# Cherenkov project
The idea and need for developing this project arose while analyzing quasi-monochromatic
Cherenkov spectral lines. While trying to improve the theoretical explanation of the
experimental results [^1], it was decided to test the Geant4 toolkit. However, the Geant4
failed in that task (discussed in [^2][^3]) because, as we later found in the source code,
the Geant4 Cherenkov process (namely `G4Cerenkov`) is based on the standard Frank-Tamm
theory of Cherenkov radiation, that is developed for ideal (infinitely thick) radiators.
However, thin radiators with frequency dispersion are necessary to obtain Cherenkov
spectral lines.

After discussing and analyzing the experimental and theoretical results, it was decided
to prepare this code and solve the Geant4 problem. One of the motives to do so is that
Geant4 would allow us to use various processes (e.g., multiple scattering), which are
not considered in the theoretical approach we used in the past (the polarization
currents method [^4]). Such processes should be vital if we should use the technique
for the proposed beam diagnostics [^5][^6].

The project comprises two targets: a library target containing the whole code
about the reworked Cherenkov process and an executable target to which the
library is statically linked. The executable target can be considered as a
standard Geant4 application.

## Cherenkov application executable (`ChR_app_exe`) target
The executable part of the project is more or less a standard Geant4 application.
That means a detector construction, a physics list, and an action initialization
objects are created. Also, some other user classes were inherited and used to
obtain data according to the experimental geometry [^1].
Besides the standard part of the executable target, one might be interested in
a few additional provided features:
- ***Loading material properties***: In `DetectorConstruction.hpp`, one can find
templates that can be used for quick loading of material properties data, either
from data files or using a specific equation (e.g., provided via a lambda function)

- ***Units and time benchmark***: In `UnitsAndBench.hpp`, one can find some unit literals
(in addition to the Geant4 system of units) for those who prefer using literals.
Also, the header has a short template class that can be utilized for time
benchmarking, preferably of specific scopes.

- ***Process .csv data***: The whole `ProcessCsvData.hpp` is based on variadic templates,
while the main idea is to enable users to quickly process any Geant4 data (ntuples
generated using `G4AnalysisManager`), despite knowing how to use the root or not.

## Cherenkov process library (`ChR_process_lib`) target
As it is planned to add the code from this library into one of the following Geant4
official releases, the whole library target is written using the Geant4 coding style.
Here, only some library features are provided, while one can find more information
through the comments in the code and article [^7]. Some more significant features:

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
tables (see more detailed explanation in [^7]).

- ***Accessing physics data***: To achieve the main improvement previously mentioned, it
is necessary to access stored vectors of the `G4PhysicsVector` class. Luckily, those
vectors are protected, and using `reinterpret_cast` can solve the problem. To
understand the source of the problem, see [^7].

- ***G4StandardCherenkovProcess***: The class is loaded through the
`G4OpticalPhysics_option1`, and the main idea of the class is to boost the code
performance. In general, the class works almost identically to `G4StandardChR_Model`,
while it can improve the performance for complex detectors (if there are many
logical volumes). This class should be used if the standard Cherenkov radiation
model is the only model utilized in simulations (for most users' needs).

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

[^2]: B. Đurnić, A. Potylitsyn, A. Bogdanov, Geant4 simulations of Cherenkov radiation
spectral lines. Comparison with experimental and theoretical results., Phys. Part. Nucl.
54(6) (2023), 1142-1151; DOI: [10.1134/S1063779623060278](https://link.springer.com/article/10.1134/S1063779623060278)

[^3]: B. Đurnić, A. Potylitsyn, A. Bogdanov, S. Gogolev, Geant4 simulation of monochromatic
Cherenkov radiation in thin quartz targets for different experimental conditions, J. Phys.:
Conference Series 2701 (2024) 012019; DOI: [10.1088/1742-6596/2701/1/012019](https://iopscience.iop.org/article/10.1088/1742-6596/2701/1/012019)

[^4]: A.P. Potylitsyn, S.Yu. Gogolev, Vavilov-Cherenkov radiation in an inclined
dielectric plate and violation of azimuthal symmetry, Phys. Part. Nucl. Lett. 16(2) (2019)
127-132; DOI: [10.1134/S1547477119020110](https://link.springer.com/article/10.1134/S1547477119020110)

[^5]: B. Đurnić, A. Potylitsyn, A. Bogdanov, S. Gogolev, Radiator thickness and its
effects on Cherenkov spectral lines, Nucl. Inst. Meth. A 1059 (2023) 169015;
DOI: [10.1016/j.nima.2023.169015](https://www.sciencedirect.com/science/article/pii/S016890022301015X)

[^6]: B. Đurnić, A. Potylitsyn, A. Bogdanov, S. Gogolev, Feasibility of using optical
Cherenkov radiation for non-relativistic ion beam diagnostics, JINST 19 (2024) C06015;
DOI: [10.1088/1748-0221/19/06/C06015](https://iopscience.iop.org/article/10.1088/1748-0221/19/06/C06015)

[^7]: B. Đurnić, A. Potylitsyn, A. Bogdanov, S. Gogolev, On the rework and
development of new Geant4 Cherenkov models, arXiv:2409.20411 [physics.acc-ph] (2024);
DOI: [10.48550/arXiv.2409.20411](https://doi.org/10.48550/arXiv.2409.20411)