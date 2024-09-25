//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//g4 headers
#include "G4CherenkovProcess.hh"
#include "G4CherenkovProcess_Messenger.hh"
#include "G4AccessPhysicsVectors.hh"
#include "G4SystemOfUnits.hh"
#include "G4StandardChR_Model.hh"
#include "G4ThinTargetChR_Model.hh"

//=========public G4CherenkovProcess:: methods=========

G4CherenkovProcess::G4CherenkovProcess(const G4String& name)
: G4VDiscreteProcess(name, fElectromagnetic) {
	p_ChRProcessMessenger = new G4CherenkovProcess_Messenger{ this };
	if (AddNewChRModel(new G4StandardChR_Model{}) && verboseLevel > 0)
		std::cout << "G4CherenkovProcess_Messenger has been added to G4CherenkovProcess\n";
	if (AddNewChRModel(new G4ThinTargetChR_Model{}) && verboseLevel > 0)
		std::cout << "G4ThinTargetChR_Model has been added to G4CherenkovProcess\n";
	G4ExtraOpticalParameters::GetInstance()->ScanAndAddUnregisteredLV();
}

G4CherenkovProcess::~G4CherenkovProcess() {
	delete p_ChRProcessMessenger;
	for (G4BaseChR_Model* aModel : m_registeredModels)
		delete aModel; //didn't go with smart pointers
}

G4double G4CherenkovProcess::MinPrimaryEnergy(const G4ParticleDefinition* aParticle, const G4Material* aMaterial) {
	G4double restE = aParticle->GetPDGMass();
	G4AccessPhysicsVector* accessor = reinterpret_cast<G4AccessPhysicsVector*>(aMaterial->GetMaterialPropertiesTable()->GetProperty(kRINDEX));
	if (!accessor)
		accessor = reinterpret_cast<G4AccessPhysicsVector*>(aMaterial->GetMaterialPropertiesTable()->GetProperty(kREALRINDEX));
	if (!accessor) {
		std::ostringstream err;
		err << "Material " << std::quoted(aMaterial->GetName()) << " has no registered RIndex property, i.e., no min energy!\n";
		G4Exception("G4CherenkovProcess::MinPrimaryEnergy", "WE_ChRProcess03", JustWarning, err);
		return -1.;
	}
	G4double maxN = accessor->GetRealDataVectorMax();
	G4double value = restE / std::sqrt(1 - 1 / (maxN * maxN));
	if (verboseLevel > 0)
		std::cout << "For particle " << std::quoted(aParticle->GetParticleName()) << ", minimum energy required for\n"
		<< "producing Cherenkov photons in material " << std::quoted(aMaterial->GetName()) << " is " << value / MeV << " MeV\n";
	return value;
}

void G4CherenkovProcess::DumpInfo() const {
	std::cout.fill('=');
	std::cout << std::setw(116) << '\n';
	std::cout << "Begin of G4CherenkovProcess::DumpInfo():\n\n"
		<< "A class G4CherenkovProcess is made as a wrapper for various Cherenkov radiation models. All\n"
		<< "Cherenkov models are stored in a std::vector<G4BaseChR_Model*>. Depending on user-loaded data,\n"
		<< "the class selects what Cherenkov models will be executed during the runtime.\n"
		<< "Currently loaded Cherenkov models are:\n";
	size_t temp = 0;
	for (auto* aModel : m_registeredModels) {
		std::cout.fill('+');
		std::cout << std::setw(21) << '\n';
		std::cout << "Model ID #" << temp++ << ": " << typeid(*aModel).name() << '\n';
		aModel->DumpModelInfo();
	}
	if (temp == 0)
		std::cout << "You still haven't registered any models! Without a registered process, you won't be able to use Cherenkov radiation in simulations!\n";
	else
		std::cout << std::setw(21) << '\n';
	std::cout << "If you want to read about what Cherenkov process is, use \"G4CherenkovProcess::ProcessDescription()\" method\n"
		<< "End of G4CherenkovProcess::DumpInfo()\n";
	std::cout.fill('=');
	std::cout << std::setw(116) << '\n';
	std::cout << std::endl;
}

void G4CherenkovProcess::ProcessDescription(std::ostream& outStream) const {
	outStream.fill('=');
	outStream << std::setw(116) << '\n';
	outStream << "Begin of G4CherenkovProcess::ProcessDescription():\n\n"
		<< "The Cherenkov radiation was discovered in 1934 by S.I. Vavilov and P.A. Cherenkov (Vavilov's student).\n"
		<< "The first theoretical explanation was provided in 1937 by I.M. Frank and I.E. Tamm. According to our\n"
		<< "understanding, when a charged particle moves through a medium faster than the phase velocity of light,\n"
		<< "photons of a wide energy range (predominantly in the optical region) are emitted. \"Moves faster\" means\n"
		<< "the condition \"beta * n >= 1\", where \"beta\" is a relativistic reduced velocity (= v / c) and \"n\" is\n"
		<< "the refractive index, is satisfied.\n\n"
		<< "In order to use this class in Geant4, one must define the refractive index of a material through the class\n"
		<< "G4MaterialPropertiesTable. To do so, one needs to provide a corresponding key index that can be found in\n"
		<< "\"G4MaterialPropertiesIndex.hh\" (just remove \"k\" from the enum). Also, note that it's advisable to\n"
		<< "define absorption always; otherwise an optical photon might enter an infinite loop of total internal\n"
		<< "reflections.\n\n"
		<< "To see more details about this specific C++ class, use the G4CherenkovProcess::DumpInfo() method.\n\n"
		<< "Some English literature I know of that one might find interesting and is a nice summary of everything:\n"
		<< "1. J.V. Jelley, Cherenkov radiation and its Applications, Pergamon Press, New York, 1958\n"
		<< "2. B.M. Bolotovskii \"Vavilov-Cherenkov radiation: its discovery and application\" Phys. Usp. 52(11), (2009) 1099-1110\n"
		<< "3. A.P. Kobzev \"On the radiation mechanism of a uniformly moving charge\", Phys. Part. Nucl. 45(3), (2014) 628-653\n\n"
		<< "End of G4CherenkovProcess::ProcessDescription()\n";
	outStream << std::setw(116) << '\n';
	outStream << std::endl;
}