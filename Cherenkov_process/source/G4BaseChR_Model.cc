//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//G4 headers
#include "G4BaseChR_Model.hh"
#include "G4AccessPhysicsVectors.hh"
#include "SomeGlobalNamespace.hh"
#include "G4Track.hh"
#include "G4Material.hh"
#include "G4LossTableManager.hh"
#include "G4OpticalParameters.hh"
#include "G4SystemOfUnits.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ExtraOpticalParameters.hh"
//std:: headers
#include <algorithm>

#define pow2(x) ((x) * (x))

//two static helper methods based around this translation unit - definition at the end of the file
static void PrintSimpleTables(const G4Material*, const std::vector<G4ChRPhysTableData::G4AroundBetaBasedValues>&);
static void PrintMoreComplexTables(const unsigned char, const G4Material*, const G4ChRPhysTableData&);

G4BaseChR_Model::G4ChRPhysicsTableVector G4BaseChR_Model::m_ChRPhysDataVec{};
unsigned int G4BaseChR_Model::m_noOfBetaSteps = 20;

//=========public G4BaseChR_Model:: methods=========

G4BaseChR_Model::G4BaseChR_Model(const char* name, const unsigned char verboseLevel)
: m_ChRModelName(name), m_verboseLevel(verboseLevel),
m_includeFiniteThickness(false), m_useModelWithEnergyLoss(false) {
	p_particleChange = new G4ParticleChange{};
}

G4BaseChR_Model::~G4BaseChR_Model() {
	delete p_particleChange;
}

//I kept the method very similar to the original G4Cerenkov method. Still, I removed bits of the code that will never execute
G4double G4BaseChR_Model::PostStepModelIntLength(const G4Track& aTrack, G4double, G4ForceCondition* condition) {
	*condition = NotForced;
	G4double stepLimit = DBL_MAX;
	size_t matIndex = aTrack.GetMaterial()->GetIndex();

	if (m_ChRPhysDataVec.size() != G4Material::GetNumberOfMaterials()) {
		const char* err = "Not all materials have been registered in Cherenkov physics tables!\n";
		G4Exception("G4BaseChR_Model::PostStepModelIntLength", "FE_BaseChR01", FatalException, err);
	}
	G4ChRPhysTableData& physData = m_ChRPhysDataVec[matIndex];
	if (physData.m_aroundBetaValues.size() <= 1)
		return stepLimit;
	
	const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
	const G4MaterialCutsCouple* couple = aTrack.GetMaterialCutsCouple();

	G4double kineticEnergy = aParticle->GetKineticEnergy();
	const G4ParticleDefinition* particleType = aParticle->GetDefinition();
	G4double mass = particleType->GetPDGMass();

	G4double beta = aParticle->GetTotalMomentum() / aParticle->GetTotalEnergy();
	G4double gamma = aParticle->GetTotalEnergy() / mass;
	G4double gammaMin = 1 / std::sqrt(1. - pow2(physData.m_aroundBetaValues.front().m_betaValue));

	if (beta <= physData.m_aroundBetaValues.front().m_betaValue)
		return stepLimit;

	G4double kinEmin = mass * (gammaMin - 1.);
	G4double RangeMin = G4LossTableManager::Instance()->GetRange(particleType, kinEmin, couple);
	G4double Range = G4LossTableManager::Instance()->GetRange(particleType, kineticEnergy, couple);
	G4double step = Range - RangeMin;

	// If the step is smaller than G4ThreeVector::getTolerance(), it may happen
	// that the particle does not move. See bug 1992.
	if (step < G4ThreeVector::getTolerance())
		return stepLimit;

	if (step < stepLimit)
		stepLimit = step;

	G4OpticalParameters* optParameters = G4OpticalParameters::Instance();
	// If user has defined an average maximum number of photons to be generated in
	// a Step, then calculate the Step length for that number of photons.
	if (optParameters->GetCerenkovMaxPhotonsPerStep() > 0) {
		const G4double charge = aParticle->GetDefinition()->GetPDGCharge();
		G4double meanNumberOfPhotons = CalculateAverageNumberOfPhotons(charge, beta, matIndex);
		step = 0.;
		if (meanNumberOfPhotons > 0.0)
			step = optParameters->GetCerenkovMaxPhotonsPerStep() / meanNumberOfPhotons;
		if (step > 0. && step < stepLimit)
			stepLimit = step;
	}

	// If user has defined an maximum allowed change in beta per step
	if (optParameters->GetCerenkovMaxBetaChange() > 0.) {
		G4double dedx = G4LossTableManager::Instance()->GetDEDX(particleType, kineticEnergy, couple);
		G4double deltaGamma = gamma - 1. / std::sqrt(1. - pow2((1. - optParameters->GetCerenkovMaxBetaChange()) * beta));

		step = mass * deltaGamma / dedx;
		if (step > 0. && step < stepLimit)
			stepLimit = step;
	}

	*condition = StronglyForced;
	return stepLimit;
}

void G4BaseChR_Model::BuildModelPhysicsTable(const G4ParticleDefinition&) {
	std::size_t numOfMaterials = G4Material::GetNumberOfMaterials();
	if (m_ChRPhysDataVec.size() == numOfMaterials)
		return;
	m_ChRPhysDataVec = G4ChRPhysicsTableVector{}; //in case some materials were deleted - rebuilding all physics tables
	m_ChRPhysDataVec.reserve(numOfMaterials);
	for (size_t i = 0; i < numOfMaterials; i++)
		AddExoticRIndexPhysicsTable(i);
	if(m_verboseLevel > 0)
		PrintChRPhysDataVec();
}

void G4BaseChR_Model::PrintChRPhysDataVec(const unsigned char printLevel, const G4Material* aMaterial) {
	const G4MaterialTable* theMaterialTable = G4Material::GetMaterialTable();
	std::cout.fill('=');
	std::cout << std::setw(66) << '\n';
	if (aMaterial)
		std::cout << "Begin of PrintChRPhysDataVec( " << std::to_string(printLevel) << ", " << aMaterial->GetName() << " )\n\n";
	else
		std::cout << "Begin of PrintChRPhysDataVec( " << std::to_string(printLevel) << ", nullptr )\n\n";

	std::cout << "Number of emitted Cherenkov photons can be calculated as:\nconst * (leftIntegral - rightIntegral / beta^2)\n";
	if (aMaterial) { //if a specific material is selected, the method prints only about that method
		if (printLevel == 0) {
			PrintSimpleTables(aMaterial, m_ChRPhysDataVec[aMaterial->GetIndex()].m_aroundBetaValues);
			goto ExitPrintFunction;
		}
		PrintMoreComplexTables(printLevel, aMaterial, m_ChRPhysDataVec[aMaterial->GetIndex()]);
		goto ExitPrintFunction;
	}
	std::cout << "\nBuilt tables of G4BaseChR_Model are:\n\n";
	if (printLevel == 0) {
		for (size_t i = 0; i < m_ChRPhysDataVec.size(); i++)
			PrintSimpleTables((*theMaterialTable)[i], m_ChRPhysDataVec[i].m_aroundBetaValues);
		goto ExitPrintFunction;
	}
	std::cout << std::setfill('+') << std::setw(66) << '\n';
	for (size_t i = 0; i < m_ChRPhysDataVec.size(); i++) {
		PrintMoreComplexTables(printLevel, (*theMaterialTable)[i], m_ChRPhysDataVec[i]);
		std::cout << '\n' << std::setfill('+') << std::setw(66) << '\n';
	}
ExitPrintFunction:
	if (aMaterial)
		std::cout << "\nEnd of PrintChRPhysDataVec( " << std::to_string(printLevel) << ", " << aMaterial->GetName() << " )\n";
	else
		std::cout << "\nEnd of PrintChRPhysDataVec( " << std::to_string(printLevel) << ", nullptr )\n";
	std::cout << std::setfill('=') << std::setw(66) << '\n';
}

//=========protected G4BaseChR_Model:: methods=========

//the following method has the same idea as the original G4Cerenkov... still, the original method
//limited the usability of G4Cerenkov, so physics tables are built through betaValues now
G4double G4BaseChR_Model::CalculateAverageNumberOfPhotons(const G4double aCharge, const G4double betaValue, const size_t materialID) {
	constexpr G4double Rfact = 369.81 / (eV * cm);
	if (betaValue <= 0)
		return 0.;
	const std::vector<G4ChRPhysTableData::G4AroundBetaBasedValues>& physDataVec = m_ChRPhysDataVec[materialID].m_aroundBetaValues;
	// the following condition should never happen - it was already done in the StepLength method
	/*if (physDataVec.size() <= 1)
		return 0.;*/
	G4double deltaE, ChRRightIntPart;
	if (betaValue <= physDataVec.front().m_betaValue) {
		deltaE = 0.;
		ChRRightIntPart = 0.;
	}
	else if (betaValue >= physDataVec.back().m_betaValue) {
		deltaE = physDataVec.back().m_leftIntegralValue;
		ChRRightIntPart = physDataVec.back().m_rightIntegralValue;
	}
	else {
		size_t lowLoc = static_cast<size_t>(std::lower_bound(physDataVec.begin() + 1, physDataVec.end(), betaValue,
			[](const G4ChRPhysTableData::G4AroundBetaBasedValues& value1, const G4double value2) {return value1.m_betaValue < value2; }) - physDataVec.begin());
		deltaE = G4LinearInterpolate2D_GetY(physDataVec[lowLoc].m_leftIntegralValue, physDataVec[lowLoc - 1].m_leftIntegralValue,
			physDataVec[lowLoc].m_betaValue, physDataVec[lowLoc - 1].m_betaValue, betaValue);
		ChRRightIntPart = G4LinearInterpolate2D_GetY(physDataVec[lowLoc].m_rightIntegralValue, physDataVec[lowLoc - 1].m_rightIntegralValue,
			physDataVec[lowLoc].m_betaValue, physDataVec[lowLoc - 1].m_betaValue, betaValue);
	}
	return Rfact * pow2(aCharge) / pow2(eplus) * (deltaE - ChRRightIntPart / pow2(betaValue));
}

//=========private G4BaseChR_Model:: methods=========

G4bool G4BaseChR_Model::AddExoticRIndexPhysicsTable(const size_t materialID, G4bool forceExoticFlag){
	const G4MaterialTable* theMaterialTable = G4Material::GetMaterialTable();
	G4ChRPhysTableData thePhysVecData{};
	G4MaterialPropertiesTable* MPT = (*theMaterialTable)[materialID]->GetMaterialPropertiesTable();
	if (!MPT) {
		m_ChRPhysDataVec.push_back(std::move(thePhysVecData));
		return false;
	}
	G4AccessPhysicsVector* RIndex = reinterpret_cast<G4AccessPhysicsVector*>(MPT->GetProperty(kRINDEX));
	if (!RIndex) //NOTE: I'm not sure if those are left separate on purpose!
		RIndex = reinterpret_cast<G4AccessPhysicsVector*>(MPT->GetProperty(kREALRINDEX));
	if (!RIndex) {
		m_ChRPhysDataVec.push_back(std::move(thePhysVecData));
		return false;
	}
	const std::vector<G4double>& RIVector = RIndex->GetDataVector();
	G4double nMax = RIndex->GetRealDataVectorMax();
	G4double nMin = RIndex->GetRealDataVectorMin();
	if (nMax <= 1.) {
		m_ChRPhysDataVec.push_back(std::move(thePhysVecData));
		return false;
	}
	const std::vector<G4double>& energyVec = RIndex->GetBinVector();
	G4double betaLowLimit = 1 / nMax;
	G4double betaHighLimit = 1 / nMin;
	// for the following, the radiation can either be produced, or not
	if (nMax == nMin) {
		G4double deltaE = energyVec.back() - energyVec.front();
		G4double ChRIntensity = deltaE / nMax;
		thePhysVecData.m_aroundBetaValues.reserve(2);
		thePhysVecData.m_aroundBetaValues.emplace_back(betaLowLimit, deltaE, ChRIntensity);
		thePhysVecData.m_aroundBetaValues.emplace_back(betaLowLimit, deltaE, ChRIntensity);
		m_ChRPhysDataVec.push_back(std::move(thePhysVecData));
		return false;
	}
	if (!forceExoticFlag) {
		const G4Material* aMaterial = (*theMaterialTable)[materialID];
		G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
		auto extraOpParams = G4ExtraOpticalParameters::GetInstance();
		for (const auto* i : *lvStore) {
			// comparing material memory addresses
			if (i->GetMaterial() == aMaterial && extraOpParams->FindChRMatData(i)->GetExoticRIndex()) {
				forceExoticFlag = true;
				break;
			}
		}
	}
	G4double deltaBeta = (betaHighLimit - betaLowLimit) / m_noOfBetaSteps;
	thePhysVecData.m_aroundBetaValues.reserve((size_t)m_noOfBetaSteps + 1);
	for (size_t j = 0; j <= m_noOfBetaSteps; j++) {
		G4double deltaE = 0.;
		G4double ChRIntensity = 0.;
		G4double beta = betaLowLimit + (G4double)j * deltaBeta;
		std::vector<std::pair<G4double, G4double>>* aCDFValues = nullptr;
		std::vector<G4ThreeVector>* bigBetaVector = nullptr;
		if (forceExoticFlag) {
			aCDFValues = new std::vector<std::pair<G4double, G4double>>{};
			aCDFValues->reserve(energyVec.size());
			aCDFValues->emplace_back(energyVec.front(), 0.);
		}
		if (j == m_noOfBetaSteps && forceExoticFlag) {
			bigBetaVector = new std::vector<G4ThreeVector>{};
			bigBetaVector->reserve(energyVec.size());
			bigBetaVector->emplace_back(energyVec.front(), 0., 0.);
		}
		for (size_t k = 1; k < RIVector.size(); k++) {
			//the following two are for the ChR condition
			G4double preTemp = beta * RIVector[k - 1];
			G4double postTemp = beta * RIVector[k];
			if (preTemp >= 1. && postTemp >= 1.) {
				ChRIntensity += (energyVec[k] - energyVec[k - 1]) * 0.5 *
					(1 / pow2(RIVector[k]) + 1 / pow2(RIVector[k - 1]));
				deltaE += energyVec[k] - energyVec[k - 1];
			}
			else if (preTemp < 1. && postTemp >= 1.) {
				//boundary condition: RIndex = 1 / beta
				G4double tempE = G4LinearInterpolate2D_GetX(RIVector[k - 1], RIVector[k], energyVec[k - 1], energyVec[k], 1 / beta);
				ChRIntensity += (energyVec[k] - tempE) * 0.5 *
					(1 / (pow2(1 / beta)) + 1 / (pow2(RIVector[k])));
				deltaE += energyVec[k] - tempE;
			}
			else if (preTemp >= 1. /*&& postTemp < 1.*/) {
				//boundary condition: RIndex = 1 / beta
				G4double tempE = G4LinearInterpolate2D_GetX(RIVector[k - 1], RIVector[k], energyVec[k - 1], energyVec[k], 1 / beta);
				ChRIntensity += (tempE - energyVec[k - 1]) * 0.5 *
					(1 / pow2(1 / beta) + 1 / pow2(RIVector[k - 1]));
				deltaE += tempE - energyVec[k - 1];
			}
			if (aCDFValues)
				aCDFValues->emplace_back(energyVec[k], deltaE - ChRIntensity / pow2(beta));
			if (bigBetaVector)
				bigBetaVector->emplace_back(energyVec[k], deltaE, ChRIntensity);
		}
		thePhysVecData.m_aroundBetaValues.emplace_back(beta, deltaE, ChRIntensity);
		if (aCDFValues) {
			// first normalize to [0., 1.] and then pass the pointer
			for (auto& i : *aCDFValues) {
				if (aCDFValues->back().second != 0) // just for the betaMin vector
					i.second /= aCDFValues->back().second;
			}
			thePhysVecData.m_aroundBetaValues.back().p_valuesCDF = aCDFValues;
		}
		if (bigBetaVector)
			thePhysVecData.p_bigBetaCDFVector = bigBetaVector;
	}
	if (m_ChRPhysDataVec.size() == materialID)
		m_ChRPhysDataVec.push_back(std::move(thePhysVecData));
	else
		m_ChRPhysDataVec[materialID] = std::move(thePhysVecData);
	return true;
}

void G4BaseChR_Model::RemoveExoticRIndexPhysicsTable(const size_t materialID) {
	try {
		// error conditions should never happen... anyway, we are in G4State_Idle, so a few processor cycles won't hurt
		G4ChRPhysTableData& thePhysVecData = m_ChRPhysDataVec.at(materialID);

		delete thePhysVecData.p_bigBetaCDFVector;
		thePhysVecData.p_bigBetaCDFVector = nullptr;

		for (auto& i : thePhysVecData.m_aroundBetaValues) {
			if (!i.p_valuesCDF) {
				std::string err{ "No ChR exotic physics table data found while the m_exoticRIndex flag is 'true'!\n" };
				err += "This is a problem in the logic of the code, please report the issue!\n";
				throw err;
			}
			delete i.p_valuesCDF;
			i.p_valuesCDF = nullptr;
		}
	}
	catch (std::out_of_range) {
		std::ostringstream err;
		err << "The material with index " << materialID << " is not found in the built physics tables!\n";
		G4Exception("G4BaseChR_Model::RemoveExoticRIndexPhysicsTable", "FE_BaseChR02", FatalException, err);
	}
	catch (const std::string err) {
		G4Exception("G4BaseChR_Model::RemoveExoticRIndexPhysicsTable", "FE_BaseChR03", FatalException, err.c_str());

	}
}

//=========static methods of translation unit=========

static void PrintSimpleTables(const G4Material* aMaterial, const std::vector<G4ChRPhysTableData::G4AroundBetaBasedValues>& theCurrentData) {
	const size_t tableSize = theCurrentData.size();
	std::cout << std::right << std::setfill('_') << std::setw(52) << '\n'
		<< "| Material: " << std::setfill(' ') << std::setw(40) << "|\n"
		<< "| " << std::setw(48) << std::left << aMaterial->GetName() << "|\n"
		<< '|' << std::right << std::setfill('_') << std::setw(51) << "|\n";
	if (tableSize <= 1) {
		std::cout << "| No physics table has been built for this" << std::setfill(' ') << std::setw(10) << "|\n"
			<< "| material (n = 1 or no n(E))" << std::setw(23) << "|\n"
			<< '|' << std::right << std::setfill('_') << std::setw(51) << "|\n";
		return;
	}
	std::cout << "|  Beta  | Left ChR integral | Right ChR integral |\n"
		<< '|' << std::setw(9) << '|' << std::setw(20) << '|' << std::setw(22) << "|\n";
	std::cout.fill(' ');
	for (size_t i = 0; i < tableSize; i++) {
		std::cout << '|' << std::setprecision(4) << std::fixed << std::setw(7) << theCurrentData[i].m_betaValue << " |"
			<< std::scientific << std::setw(15) << theCurrentData[i].m_leftIntegralValue << std::setw(5) << '|'
			<< std::setw(15) << theCurrentData[i].m_rightIntegralValue << std::setw(7) << "|\n";
	}
	std::cout << '|' << std::setfill('_') << std::setw(9) << '|' << std::setw(20) << '|' << std::setw(22) << "|\n";
}

static void PrintMoreComplexTables(const unsigned char printLevel, const G4Material* aMaterial, const G4ChRPhysTableData& theCurrentData) {
	const size_t tableSize = theCurrentData.m_aroundBetaValues.size();
	if (tableSize <= 1) {
		std::cout << "No BaseChR_Model physics tables were found for material "
			<< std::quoted(aMaterial->GetName()) << "\nThat means the refractive index of the material is '1', or it is not defined!\n";
		return;
	}
	if (!theCurrentData.m_aroundBetaValues.front().p_valuesCDF) {
		std::cout << "Material " << std::quoted(aMaterial->GetName()) << " has only standard BaseChR_Model\n"
			<< "physics tables built (no physics tables for exotic refractive indices were found):\n";
		PrintSimpleTables(aMaterial, theCurrentData.m_aroundBetaValues);
		return;
	}
	std::cout << "Material: " << std::quoted(aMaterial->GetName())
		<< "\nBaseChR_Model physics tables for exotic refractive indices found!\n"
		<< "Standard physics tables are:\n";
	PrintSimpleTables(aMaterial, theCurrentData.m_aroundBetaValues);
	std::cout << "\nCDF tables between betaMin and betaMax are:\n";
	size_t noOfColumns = 8; // going with 8 max possible no of columns per table
	size_t remainder = tableSize % noOfColumns; // no of elements not in tables
	size_t noOfTables = tableSize / noOfColumns; // no of table parts
	if (remainder != 0) // if I can't place all elements in tables, no of tables ++
		noOfTables++;
	noOfColumns = tableSize / noOfTables; // normal number of columns per tables
	remainder = tableSize % noOfColumns; // no of tables with an extra column
	auto nextBeginIter = theCurrentData.m_aroundBetaValues.begin();
	size_t noOfRIndexValues = theCurrentData.m_aroundBetaValues.front().p_valuesCDF->size();
	std::cout.fill('_');
	for (size_t i = 0; i < noOfTables; i++) {
		std::vector<G4ChRPhysTableData::G4AroundBetaBasedValues>::const_iterator progressiveIter;
		size_t nextEnd = noOfColumns;
		if (remainder != 0) {
			remainder--;
			nextEnd++;
		}
		//following setw(NUMBER), where NUMBER = nextEnd * " | x.xxxx" + "| x.xxx" + " |\n" = nextEnd * 9 + 10
		std::cout << std::setw(nextEnd * 9 + 10) << '\n';
		std::cout.fill(' ');
		std::cout << "|  ChR  |" << std::right
			<< std::setw((nextEnd * 9 + 1) / 2 - 2) << "beta" << std::setw(nextEnd * 9 - ((nextEnd * 9 + 1) / 2 - 3)) << "|\n";
		std::cout << "| photon|" << std::setfill('-') << std::setw(nextEnd * 9 + 1) << "|\n";
		std::cout << "| E [eV]|";
		progressiveIter = nextBeginIter;
		std::cout.precision(4);
		for (; progressiveIter != nextBeginIter + nextEnd; progressiveIter++)
			std::cout << ' ' << std::fixed << progressiveIter->m_betaValue << " |";
		std::cout << "\n|" << std::setfill('_') << std::setw(8) << '|';
		for (size_t k = 0; k < nextEnd; k++)
			std::cout << std::setw(9) << '|';
		std::cout << '\n';
		for (size_t j = 0; j < noOfRIndexValues; j++) {
			progressiveIter = nextBeginIter;
			std::cout << "| " << std::setprecision(3) << (*theCurrentData.m_aroundBetaValues.front().p_valuesCDF)[j].first / eV;
			std::cout.precision(4);
			for (; progressiveIter != nextBeginIter + nextEnd; progressiveIter++)
				std::cout << " | " << (*(*progressiveIter).p_valuesCDF)[j].second;
			std::cout << " |\n";
		}
		nextBeginIter = progressiveIter;
		std::cout << '|' << std::setw(8) << "|";
		for (size_t k = 0; k < nextEnd; k++)
			std::cout << std::setw(9) << '|';
		std::cout << std::endl;
	}
	if (printLevel < 2)
		return;
	std::cout << "\nCDF for higher than betaMax is calculated based on the following values:\n";
	std::cout << std::setfill('_') << std::setw(52) << '\n';
	std::cout << "| E [eV] | Left ChR integral | Right ChR integral |\n"
		<< '|' << std::setw(9) << '|' << std::setw(20) << '|' << std::setw(22) << "|\n";
	std::cout.fill(' ');
	for (const auto& i : *theCurrentData.p_bigBetaCDFVector) {
		std::cout << '|' << std::setprecision(4) << std::fixed << std::setw(7) << i.getX() / eV << " |"
			<< std::scientific << std::setw(15) << i.getY() << std::setw(5) << '|'
			<< std::setw(15) << i.getZ() << std::setw(7) << "|\n";
	}
	std::cout << '|' << std::setfill('_') << std::setw(9) << '|' << std::setw(20) << '|' << std::setw(22) << "|\n";
}