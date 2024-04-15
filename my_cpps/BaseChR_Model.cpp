#include "BaseChR_Model.hpp"

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

beginChR

//this can be done with verbose as well, but preprocessor removes it from the code while with verbose everything stays
#if 0 //for tracking without Debug
#define BaseChR_test
static std::atomic<int> localInstanceCounter = 0; //the idea is to set '#if 1' only for a single thread trying, but still... atomic
#endif

#define pow2(x) (x * x)

std::vector<BaseChR_Model::PhysicsTableData> BaseChR_Model::m_ChRPhysDataVec{};

//=========public ChR::BaseChR_Model:: methods=========
BaseChR_Model::BaseChR_Model(const char* name, unsigned char val)
: m_ChRModelName(name),
m_includeFiniteThickness(false), m_useModelWithEnergyLoss(false),
p_G4OptParameters(G4OpticalParameters::Instance()),
m_noOfBetaSteps(val), m_verboseLevel(0),
m_Emin(0.), m_Emax(0.), m_noOfPhotons(0) { //this line is to keep the compiler silent
	p_particleChange = new G4ParticleChange{};

#ifdef BaseChR_test
	std::cout.fill('=');
	std::cout << std::setw(51) << '\n'
		<< "Constructor BaseChR_Model::BaseChR_Model(...) #" << ++localInstanceCounter << '\n'
		<< std::setw(51) << '\n';
#endif // BaseChR_test

}

BaseChR_Model::~BaseChR_Model() {
	delete p_particleChange;

#ifdef BaseChR_test
	std::cout.fill('=');
	std::cout << std::setw(51) << '\n'
		<< "Destructor BaseChR_Model::BaseChR_Model(...) #" << localInstanceCounter-- << '\n'
		<< std::setw(51) << '\n';
#endif // BaseChR_test

}

double BaseChR_Model::PostStepModelIntLength(const G4Track& aTrack, double previousStepSize, G4ForceCondition* condition) {
	//I kept the method very similar to the original G4Cerenkov method. Still, I removed bits of the code that will never execute

#ifdef BaseChR_test
	std::cout.fill('=');
	std::cout << std::setw(51) << '\n'
		<< "Begin of BaseChR_Model::PostStepModelIntLength(...) for particle:" << '\n'
		<< aTrack.GetParticleDefinition()->GetParticleName() << " of kinetic energy: " << aTrack.GetKineticEnergy() / MeV << " MeV\n";
	std::cout.fill(' ');
#endif // BaseChR_test

	*condition = NotForced;
	double stepLimit = DBL_MAX;
	m_noOfPhotons = 0;
	G4Material* aMaterial = aTrack.GetMaterial();
	size_t matIndex = aMaterial->GetIndex();

#ifdef BaseChR_test
		std::cout << "Current material: " << std::left << std::setw(20) << aMaterial->GetName() << " Material index: " << aMaterial->GetIndex() << '\n' << std::right;
#endif // BaseChR_test

	if (m_ChRPhysDataVec.size() != G4Material::GetNumberOfMaterials()){

#ifdef BaseChR_test
		std::cout << "End of BaseChR_Model::PostStepModelIntLength(...)\n";
		std::cout.fill('=');
		std::cout << std::setw(51) << '\n';
#endif // BaseChR_test

		return stepLimit;
	}
	//The following happens if RIndex == 1. or if there's no RIndex defined... or if the user changes m_noOfBetaSteps to 0...
	//It's just for safety if CherenkovProcess class doesn't check it previously.
	if (m_ChRPhysDataVec[matIndex].betaVector.size() <= 1) {

#ifdef BaseChR_test
		std::cout << "End of BaseChR_Model::PostStepModelIntLength(...)\n";
		std::cout.fill('=');
		std::cout << std::setw(51) << '\n';
#endif // BaseChR_test

		return stepLimit;
	}
	const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
	const G4MaterialCutsCouple* couple = aTrack.GetMaterialCutsCouple();

	double kineticEnergy = aParticle->GetKineticEnergy();
	const G4ParticleDefinition* particleType = aParticle->GetDefinition();
	double mass = particleType->GetPDGMass();

	double beta = aParticle->GetTotalMomentum() / aParticle->GetTotalEnergy();
	double gamma = aParticle->GetTotalEnergy() / mass;
	double gammaMin = 1 / std::sqrt(1. - pow2(m_ChRPhysDataVec[matIndex].betaVector.front()));

#ifdef BaseChR_test
	std::cout << "Beta: " << beta << "\tGamma: " << gamma << '\n';
#endif // BaseChR_test

	if (beta <= m_ChRPhysDataVec[matIndex].betaVector.front()) {

#ifdef BaseChR_test
		std::cout << "End of BaseChR_Model::PostStepModelIntLength(...)\n";
		std::cout.fill('=');
		std::cout << std::setw(51) << '\n';
#endif // BaseChR_test

		return stepLimit;
	}

	double kinEmin = mass * (gammaMin - 1.);
	double RangeMin = G4LossTableManager::Instance()->GetRange(particleType, kinEmin, couple);
	double Range = G4LossTableManager::Instance()->GetRange(particleType, kineticEnergy, couple);
	double step = Range - RangeMin;

#ifdef BaseChR_test
	std::cout << "RangeMin: " << RangeMin / um << " um\tRange: " << Range / um << " um\tStep: " << step / um << " um\n";
#endif // BaseChR_test

	// If the step is smaller than G4ThreeVector::getTolerance(), it may happen
	// that the particle does not move. See bug 1992.
	if (step < G4ThreeVector::getTolerance()) {

#ifdef BaseChR_test
		std::cout << "End of BaseChR_Model::PostStepModelIntLength(...)\n";
		std::cout.fill('=');
		std::cout << std::setw(51) << '\n';
#endif // BaseChR_test

		return stepLimit;
	}

	if (step < stepLimit)
		stepLimit = step;

	// If user has defined an average maximum number of photons to be generated in
	// a Step, then calculate the Step length for that number of photons.
	if (p_G4OptParameters->GetCerenkovMaxPhotonsPerStep() > 0) {
		const double charge = aParticle->GetDefinition()->GetPDGCharge();
		double meanNumberOfPhotons = CalculateAverageNumberOfPhotons(charge, beta, aMaterial);
		step = 0.;
		if (meanNumberOfPhotons > 0.0)
			step = p_G4OptParameters->GetCerenkovMaxPhotonsPerStep() / meanNumberOfPhotons;
		if (step > 0. && step < stepLimit)
			stepLimit = step;

#ifdef BaseChR_test
		std::cout << "Step from max No of photons limit: " << step / um << " um\n";
#endif // BaseChR_test

	}

	// If user has defined an maximum allowed change in beta per step
	if (p_G4OptParameters->GetCerenkovMaxBetaChange() > 0.) {
		double dedx = G4LossTableManager::Instance()->GetDEDX(particleType, kineticEnergy, couple);
		double deltaGamma = gamma - 1. / std::sqrt(1. - pow2((1. - p_G4OptParameters->GetCerenkovMaxBetaChange()) * beta));

		step = mass * deltaGamma / dedx;
		if (step > 0. && step < stepLimit)
			stepLimit = step;

#ifdef BaseChR_test
		std::cout << "Step from max beta change: " << step / um << " um\n";
#endif // BaseChR_test

	}

#ifdef BaseChR_test
	std::cout << "End of BaseChR_Model::PostStepModelIntLength(...)\n";
	std::cout.fill('=');
	std::cout << std::setw(51) << '\n';
#endif // BaseChR_test

	*condition = StronglyForced;
	return stepLimit;
}

void BaseChR_Model::PrepareModelPhysicsTable(const G4ParticleDefinition&) {
	//nothing to do here unless the user overrides
}

void BaseChR_Model::BuildModelPhysicsTable(const G4ParticleDefinition&) {
	const G4MaterialTable* theMaterialTable = G4Material::GetMaterialTable();
	std::size_t numOfMaterials = G4Material::GetNumberOfMaterials();
	if (m_ChRPhysDataVec.size() == numOfMaterials)
		return;
	m_ChRPhysDataVec.reserve(numOfMaterials);
	for (size_t i = m_ChRPhysDataVec.size(); i < numOfMaterials; i++) {
		PhysicsTableData thePhysVecData{};
		G4MaterialPropertiesTable* MPT = (*theMaterialTable)[i]->GetMaterialPropertiesTable();
		if (MPT) {
			AccessPhysicsVector* RIndex = reinterpret_cast<AccessPhysicsVector*>(MPT->GetProperty(kRINDEX));
			if (!RIndex)
				RIndex = reinterpret_cast<AccessPhysicsVector*>(MPT->GetProperty(kREALRINDEX));
			if (RIndex) {
				const std::vector<double> energyVec = RIndex->GetBinVector();
				const std::vector<double> RIVector = RIndex->GetDataVector();
				double nMax = RIndex->GetRealDataVectorMax();
				double nMin = RIndex->GetRealDataVectorMin();
				if (nMax > 1.) {
					double betaLowLimit = 1 / nMax;
					double betaHighLimit = 1 / nMin;
					double deltaBeta = (betaHighLimit - betaLowLimit) / m_noOfBetaSteps;
					for (size_t j = 0; j <= m_noOfBetaSteps; j++) {
						double Emin = -1.;
						double Emax = -1.;
						double deltaE = 0.;
						double ChRIntensity = 0.;
						double beta = betaLowLimit + j * deltaBeta;
						for (size_t k = 1; k < RIVector.size(); k++) {
							double preTemp = beta * RIVector[k - 1];
							double postTemp = beta * RIVector[k];
							if (preTemp >= 1. && postTemp >= 1.) {
								ChRIntensity += (energyVec[k] - energyVec[k - 1]) * 0.5 *
									(1 / pow2(RIVector[k]) + 1 / pow2(RIVector[k - 1]));
								deltaE += energyVec[k] - energyVec[k - 1];
								if (k == RIVector.size() - 1) Emax = energyVec.back();
								if (Emin == -1) Emin = energyVec.front();
							}
							else if (preTemp < 1. && postTemp < 1.)
								continue;
							else if (preTemp >= 1. /*&& postTemp < 1.*/) {
								Emax = LinearInterpolate2D(RIVector[k - 1], RIVector[k], energyVec[k - 1], energyVec[k], 0., 1 / beta);
								ChRIntensity += (Emax - energyVec[k - 1]) * 0.5 *
									(1 / pow2(1 / beta) + 1 / pow2(RIVector[k - 1]));
								deltaE += Emax - energyVec[k - 1];
								if (Emin == -1) Emin = energyVec.front();
							}
							else /*(preTemp < 1. && postTemp >= 1.)*/ {
								double tempE = LinearInterpolate2D(RIVector[k - 1], RIVector[k], energyVec[k - 1], energyVec[k], 0., 1 / beta);
								ChRIntensity += (energyVec[k] - tempE) * 0.5 *
									(1 / (pow2(1 / beta)) + 1 / (pow2(RIVector[k])));
								deltaE += energyVec[k] - tempE;
								if (Emin == -1) Emin = tempE;
								if (k == RIVector.size() - 1) Emax = energyVec.back();
							}
						}
						thePhysVecData.betaVector.push_back(beta);
						thePhysVecData.dataVector.push_back(ChRIntensity);
						thePhysVecData.photonEVector.emplace_back(deltaE, Emin, Emax);
					}
				}
			}
		}
		m_ChRPhysDataVec.push_back(std::move(thePhysVecData));
	}

#ifdef BaseChR_test //might use verbose in the future
	PrintChRPhysDataVec();
#endif // BaseChR_test

}

void BaseChR_Model::PrintChRPhysDataVec() const {
	const G4MaterialTable* theMaterialTable = G4Material::GetMaterialTable();
	std::cout.fill('=');
	std::cout << std::setw(76) << '\n'
		<< "Built tables of BaseChR_Model are:" << '\n';
	std::cout.fill('_');
	for (size_t i = 0; i < m_ChRPhysDataVec.size(); i++) {
		std::cout << std::setw(76) << '\n' << "|   Material: " << std::setw(60) << std::left << std::setfill(' ') << (*theMaterialTable)[i]->GetName().c_str() << "|\n"
			<< '|' << std::right << std::setfill('_') << std::setw(75) << "|\n";
		if (m_ChRPhysDataVec[i].betaVector.size() <= 1)
			std::cout << "|   No physics table has been built for this material (n = 1 or no n(E))  |\n"
			<< '|' << std::right << std::setw(75) << "|\n";
		else {
			std::cout << "|  Beta  | Right ChR integral | Left ChR integral | Emin [eV] | Emax [eV] |\n"
				<< '|' << std::setw(9) << '|' << std::setw(21) << '|' << std::setw(20) << '|' << std::setw(12) << '|' << std::setw(13) << "|\n";
			for (size_t j = 0; j < m_ChRPhysDataVec[i].betaVector.size(); j++) {
				std::cout.fill(' ');
				std::cout << '|' << std::setprecision(4) << std::fixed << std::setw(7) << m_ChRPhysDataVec[i].betaVector[j] << " |"
					<< std::scientific << std::setw(15) << m_ChRPhysDataVec[i].dataVector[j] << std::setw(6) << '|'
					<< std::setw(15) << m_ChRPhysDataVec[i].photonEVector[j].getX() << std::setw(5) << '|'
					<< std::fixed << std::setprecision(3) << std::setw(8) << m_ChRPhysDataVec[i].photonEVector[j].getY() / eV << std::setw(4) << '|'
					<< std::setw(8) << m_ChRPhysDataVec[i].photonEVector[j].getZ() / eV << std::setw(5) << "|\n";
			}
			std::cout << '|' << std::setfill('_') << std::setw(9) << '|' << std::setw(21) << '|' << std::setw(20) << '|' << std::setw(12) << '|' << std::setw(13) << "|\n";
}
		std::cout << '\n';
	}
	std::cout << "End of built tables of BaseChR_Model\n";
	std::cout.fill('=');
	std::cout << std::setw(76) << '\n';
}

void BaseChR_Model::PrepareWorkerModelPhysicsTable(const G4ParticleDefinition& aParticle) {
	PrepareModelPhysicsTable(aParticle);
}

void BaseChR_Model::BuildWorkerModelPhysicsTable(const G4ParticleDefinition& aParticle) {
	BuildModelPhysicsTable(aParticle);
}

void BaseChR_Model::DumpModelInfo() const {
	std::cout << "There's no information about the model!\n";
}

//=========protected ChR::BaseChR_Model:: methods=========
//the following method has the same idea as the original G4Cerenkov... still I prefer using beta instead of energy...
//that way I can ensure that all refrative index functions can be processed properly
double BaseChR_Model::CalculateAverageNumberOfPhotons(const double charge, const double beta, const G4Material* aMaterial) {
	constexpr double Rfact = 369.81 / (eV * cm);
	if (beta <= 0)
		return 0.;
	const std::vector<double>& betaVec = m_ChRPhysDataVec[aMaterial->GetIndex()].betaVector;
	if (betaVec.size() <= 1)
		return 0.;
	const std::vector<double>& dataVec = m_ChRPhysDataVec[aMaterial->GetIndex()].dataVector;
	const std::vector<G4ThreeVector>& energiesVec = m_ChRPhysDataVec[aMaterial->GetIndex()].photonEVector;
	double deltaE, ChRRightIntPart;
	if (beta <= betaVec.front()) {
		deltaE = 0.;
		ChRRightIntPart = 0.;
	}
	else if (beta >= betaVec.back()) {
		deltaE = energiesVec.back().getX();
		ChRRightIntPart = dataVec.back();
		m_Emin = energiesVec.back().getY();
		m_Emax = energiesVec.back().getZ();
	}
	else {
		//no need for safeties, with if and else if, problems are removed:
		size_t lowLoc = std::distance(betaVec.begin(), std::lower_bound(betaVec.begin(), betaVec.end(), beta)) - 1;
		m_Emin = energiesVec[lowLoc + 1].getY(); //just subtracted 1 previously, so this should always work in range
		m_Emax = energiesVec[lowLoc + 1].getZ(); //taking limits of higher node of the bin for energy limits
		ChRRightIntPart = LinearInterpolate2D(dataVec[lowLoc], dataVec[lowLoc + 1], betaVec[lowLoc], betaVec[lowLoc + 1], 0., beta);
		deltaE = LinearInterpolate2D(energiesVec[lowLoc].getX(), energiesVec[lowLoc + 1].getX(), betaVec[lowLoc], betaVec[lowLoc + 1], 0., beta);
	}
	return Rfact * pow2(charge) * (deltaE - ChRRightIntPart / (pow2(beta))); //no need for eplus^2, it's 1 anyway
}

endChR