ChR_project

The current version of Geant4 only supports the traditional Frank-Tamm
theory of Cherenkov radiation for ideal (infinitely thick) radiators.
Because of that, Geant4 fails to describe experimental quasi-monochromatic
Cherenkov spectral lines. To solve the problem, the code provided in this
project completely reworks the Cherenkov process and adds a model that
considers a single finite dimension of a radiator.

In the reworked code, the CherenkovProcess class is considered as a wrapper
class for various models describing Cherenkov radiation. In the current
code version, only two models are provided: AlmostOriginalChR_Model and
TammThinTarget_Model. The former model builds physics tables differently
compared to the original Geant4 model due to some inaccuracies of the
original model (possible simulation errors for more complex dependencies
of refractive index on the radiation wavelength). Also, some slight
modifications were introduced (see comments along the code). The latter
model allows one to consider a single finite dimension of a radiator.
The MyOpticalParameters class is used to help manage Cherenkov models.

The optical physics system was modified entirely to register the Cherenkov
process. Now, it is inheritance-friendly and allows one to quickly and
easily register his/her own optical processes.

Some other interesting and helpful code utilities are provided along the
project. For instance, in DetectorConstruction.hpp, one can find a template
function that helps to load material properties tables quickly. Also,
ProcessCsvData.hpp exploits variadic templates to help one promptly obtain
something useful out of .csv data obtained through G4AnalysisManager. That
way, anyone can easily read and use Geant4 data despite having experience
using the root.
