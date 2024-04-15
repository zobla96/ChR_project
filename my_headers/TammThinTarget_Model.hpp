#pragma once
#ifndef TammThinTarget_Model_hpp
#define TammThinTarget_Model_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "BaseChR_Model.hpp"
//G4 headers


beginChR

//this can be done with verbose as well, but preprocessor removes it from the code while with verbose everything stays
#if 0 //for tracking without Debug
#define TammThinTarget_test
static std::atomic<int> localTammInstanceCounter = 0; //the idea is to set '#if 1' only for a single thread trying, but still... atomic
#endif

template<typename T>
class TammThinTarget_Model : public BaseChR_Model {
public:
	static_assert(std::is_enum_v<T> == true, "The class TammThinTarget_Model<T> is designed to support enum tpye of T!");
	TammThinTarget_Model(const char* name = "TammThinTarget_Model");
	virtual ~TammThinTarget_Model();
	_NODISCARD virtual G4VParticleChange* PostStepModelDoIt(const G4Track&, const G4Step&) override;
	virtual void DumpModelInfo() const override;
private:
	const std::shared_ptr<MyOpticalParameters<T>> m_myOptParameters;
};

//=========public ChR::TammThinTarget_Model:: methods=========
template<typename T>
TammThinTarget_Model<T>::TammThinTarget_Model(const char* name)
: BaseChR_Model(name),
m_myOptParameters(MyOpticalParameters<T>::GetInstance()) {
	m_includeFiniteThickness = true;

#ifdef TammThinTarget_test
	std::cout.fill('=');
	std::cout << std::setw(51) << '\n'
		<< "Constructor TammThinTarget_Model<T>::TammThinTarget_Model(...) #" << ++localTammInstanceCounter << '\n'
		<< std::setw(51) << '\n';
#endif // TammThinTarget_test

}

template<typename T>
TammThinTarget_Model<T>::~TammThinTarget_Model() {

#ifdef TammThinTarget_test
	std::cout.fill('=');
	std::cout << std::setw(51) << '\n'
		<< "Destructor TammThinTarget_Model<T>::~TammThinTarget_Model(...) #" << localTammInstanceCounter-- << '\n'
		<< std::setw(51) << '\n';
#endif // TammThinTarget_test

}

template<typename T>
G4VParticleChange* TammThinTarget_Model<T>::PostStepModelDoIt(const G4Track& aTrack, const G4Step& aStep) {

#ifdef TammThinTarget_test
	std::cout.fill('=');
	std::cout.precision(4); //floating point precision
	std::cout << std::setw(100) << '\n';
	std::cout.fill(' '); //ASCII space -> 32
	std::cout << "Begin of TammThinTarget_Model<T>::PostStepModelDoIt for particle:\n" << std::setw(20) << std::right
		<< aTrack.GetParticleDefinition()->GetParticleName() << " of kinetic energy: " << aStep.GetPreStepPoint()->GetKineticEnergy() / MeV << " MeV\n";
#endif // TammThinTarget_test

	p_particleChange->Initialize(aTrack);

	const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
	const G4Material* aMaterial = aTrack.GetMaterial();

#ifdef TammThinTarget_test
	std::cout << "Current material: " << std::left << std::setw(20) << aMaterial->GetName() << " Material index: " << aMaterial->GetIndex() << '\n';
#endif // TammThinTarget_test

	if (m_ChRPhysDataVec[aMaterial->GetIndex()].betaVector.size() <= 1) {

#ifdef TammThinTarget_test
		std::cout << "End of TammThinTarget_Model<T>::PostStepModelDoIt(...)\n";
		std::cout.fill('=');
		std::cout << std::setw(100) << '\n';
#endif // TammThinTarget_test

		return p_particleChange;
	}
	const double matThickness = m_myOptParameters->FindChRMatData(aTrack.GetVolume()->GetLogicalVolume())->m_matThickness;
	if (matThickness <= 0) {
		const char* err = "You are trying to use a Cherenkov model for thin targets without specifing the thickness!\n";
		G4Exception("TammThinTarget_Model<T>::PostStepModelDoIt", "FE1021", FatalException, err);
	}

	const G4StepPoint* preStepPoint = aStep.GetPreStepPoint();
	const G4StepPoint* postStepPoint = aStep.GetPostStepPoint();

	const G4ThreeVector x0 = preStepPoint->GetPosition();
	const G4ThreeVector p0 = aStep.GetDeltaPosition().unit();
	const double t0 = preStepPoint->GetGlobalTime();

	G4PhysicsFreeVector* RIndex = aMaterial->GetMaterialPropertiesTable()->GetProperty(kRINDEX);
	if (!RIndex)
		RIndex = aMaterial->GetMaterialPropertiesTable()->GetProperty(kREALRINDEX);
	

	const double charge = aParticle->GetDefinition()->GetPDGCharge();
	const double beta = (preStepPoint->GetBeta() + postStepPoint->GetBeta()) * 0.5;

	double meanNumberOfPhotons = CalculateAverageNumberOfPhotons(charge, beta, aMaterial);

	if (meanNumberOfPhotons <= 0.0) {//unchanged particle; number of secondaries is 0 anyway after p_particleChange->Initialize(aTrack);

#ifdef TammThinTarget_test
		std::cout << "End of TammThinTarget_Model<T>::PostStepModelDoIt(...)\n";
		std::cout.fill('=');
		std::cout << std::setw(100) << '\n';
#endif // TammThinTarget_test

		return p_particleChange;
	}

	meanNumberOfPhotons = meanNumberOfPhotons * aStep.GetStepLength();
	m_noOfPhotons = (int)G4Poisson(meanNumberOfPhotons);

#ifdef TammThinTarget_test
	std::cout << "Number of photons: " << m_noOfPhotons << '\n';
#endif // TammThinTarget_test

	//got no idea what's the point of 'p_G4OptParameters->GetCerenkovStackPhotons()', but staying consistent...
	if (m_noOfPhotons <= 0 || !p_G4OptParameters->GetCerenkovStackPhotons()) {

#ifdef TammThinTarget_test
		std::cout << "End of TammThinTarget_Model<T>::PostStepModelDoIt(...)\n";
		std::cout.fill('=');
		std::cout << std::setw(100) << '\n';
#endif // TammThinTarget_test

		return p_particleChange;
	}
	p_particleChange->SetNumberOfSecondaries(m_noOfPhotons);

	if (p_G4OptParameters->GetCerenkovTrackSecondariesFirst()) {
		if (aTrack.GetTrackStatus() == fAlive)
			p_particleChange->ProposeTrackStatus(fSuspend);
	}

	double lossEnergy = 0.;

#ifdef TammThinTarget_test
	std::cout.fill('_');
	std::cout << std::right << std::setw(100) << '\n'
		<< "| Photon No | Energy [eV] | RIndex | Phi [deg] | Theta [deg] | ThetaInit [deg] | GaussSigma [deg] |\n"
		<< '|' << std::setw(12) << '|' << std::setw(14) << '|' << std::setw(9) << '|' << std::setw(12) << '|'
		<< std::setw(14) << '|' << std::setw(18) << '|' << std::setw(20) << "|\n";
	std::cout.fill(' ');
#endif // TammThinTarget_test

	for (G4int i = 0; i < m_noOfPhotons; ++i) {
		double rand;
		double sampledEnergy, sampledRI;
		double cosTheta, sin2Theta;
		// sample an energy
		do {
			rand = G4UniformRand();
			sampledEnergy = m_Emin + rand * (m_Emax - m_Emin);
			sampledRI = RIndex->Value(sampledEnergy);
			cosTheta = 1. / (sampledRI * beta); //might give > 1. for strange shapes of n(E)
			sin2Theta = (1.0 - cosTheta) * (1.0 + cosTheta);
		} while (sin2Theta <= 0.);
		double waveLng = 1.239841984e-6 * m * eV / sampledEnergy; //h * c = 1.239841984e-6 * m * eV
		double sinTheta = std::sqrt(sin2Theta);
		constexpr double coef = 0.75 / CLHEP::pi; // 2.78 / 2.355 = 1.1805
		//in the previous line, changed from the theoretical 1.1805 to 0.75 because the widening was too great for the experimental results
		double gaussSigma = coef * waveLng / (matThickness * sinTheta);
		double thetaChR = std::acos(cosTheta);
		
#ifdef TammThinTarget_test
		double initTheta = thetaChR;
#endif // TammThinTarget_test

		std::normal_distribution<double> thetaChRGauss{ thetaChR, gaussSigma };
		thetaChR = thetaChRGauss(g_mtGen);
		cosTheta = std::cos(thetaChR);
		sinTheta = std::sin(thetaChR);
		//change here
		rand = G4UniformRand();
		double phi = CLHEP::twopi * rand;
		double sinPhi = std::sin(phi);
		double cosPhi = std::cos(phi);

#ifdef TammThinTarget_test
		std::string photNo = "#" + std::to_string(i + 1);
		std::cout << '|' << std::setw(7) << photNo << std::setw(5) << '|'
			<< std::fixed << std::setprecision(2) << std::setw(9) << sampledEnergy / eV << std::setw(5) << '|'
			<< std::setprecision(4) << std::setw(7) << RIndex->Value(sampledEnergy) << std::setw(2) << '|'
			<< std::setprecision(2) << std::setw(8) << phi / deg << std::setw(4) << '|'
			<< std::setprecision(4) << std::setw(10) << thetaChR / deg << std::setw(4) << '|'
			<< std::setw(12) << initTheta / deg << std::setw(6) << '|'
			<< std::setw(12) << gaussSigma / deg << std::setw(8) << "|\n";
#endif // TammThinTarget_test

		G4ParticleMomentum photonMomentum(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);

		// Rotate momentum direction back to global reference system
		photonMomentum.rotateUz(p0);

		// Determine polarization of new photon
		G4ThreeVector photonPolarization(cosTheta * cosPhi, cosTheta * sinPhi, -sinTheta);

		// Rotate back to original coord system
		photonPolarization.rotateUz(p0);
		// Generate a new photon:
		auto aCerenkovPhoton = new G4DynamicParticle(G4OpticalPhoton::OpticalPhoton(), photonMomentum);

		aCerenkovPhoton->SetPolarization(photonPolarization);
		aCerenkovPhoton->SetKineticEnergy(sampledEnergy);
		lossEnergy += sampledEnergy;

		rand = G4UniformRand();

		double delta = rand * aStep.GetStepLength();
		double deltaTime = delta / (preStepPoint->GetVelocity() + rand * (postStepPoint->GetVelocity() - preStepPoint->GetVelocity()) * 0.5);

		double aSecondaryTime = t0 + deltaTime;
		G4ThreeVector aSecondaryPosition = x0 + rand * aStep.GetDeltaPosition();

		// Generate new G4Track object:
		G4Track* aSecondaryTrack = new G4Track(aCerenkovPhoton, aSecondaryTime, aSecondaryPosition);

		aSecondaryTrack->SetTouchableHandle(aStep.GetPreStepPoint()->GetTouchableHandle());
		aSecondaryTrack->SetParentID(aTrack.GetTrackID());
		p_particleChange->AddSecondary(aSecondaryTrack);
	}

#ifdef TammThinTarget_test
	std::cout.fill('_');
	std::cout << '|' << std::setw(12) << '|' << std::setw(14) << '|' << std::setw(9) << '|' << std::setw(12) << '|'
		<< std::setw(14) << '|' << std::setw(18) << '|' << std::setw(20) << "|\n";
#endif // TammThinTarget_test

	if (m_useModelWithEnergyLoss) {
		//considering only energy loss, but neglecting change in momentum direction...
		//photons have very low energy and are emitted uniformly around the particle so this should be 
		//really meager, and almost seems like IEEE754 error would be greater than the actual change.
		//Or should I maybe consider momentum direction change as well?
		p_particleChange->ProposeEnergy(p_particleChange->GetEnergy() - lossEnergy);
	}
	if (m_verboseLevel > 1)
		std::cout << "\n Exiting from AlmostOriginalChR_Model::DoIt -- NumberOfSecondaries = "
		<< p_particleChange->GetNumberOfSecondaries() << G4endl;

#ifdef TammThinTarget_test
	std::cout << "Emitted energy: " << lossEnergy / eV << " eV\nEnd of TammThinTarget_Model<T>::PostStepModelDoIt(...)\n";
	std::cout.fill('=');
	std::cout << std::setw(100) << '\n';
#endif // TammThinTarget_test

	return p_particleChange;
}

template<typename T>
void TammThinTarget_Model<T>::DumpModelInfo() const {
	std::cout << "The standard Cherenkov radiation model is shown in the 'AlmostOriginalChR_model' class. However, such a model\n"
		<< "is based on the ideal-case theory, i.e., for the case of an infinitely thick radiator. It means that\n"
		<< "there's \"enough space\" for Cherenkov radiation interference effects, so the radiation cancels itself\n"
		<< "on all angles other than those satisfying condition cos(thetaChR) = 1 / (beta * RIndex). As one can see\n"
		<< "from the condition, that means that photons of a specific wavelength are emitted along the thetaChR angle,\n"
		<< "which is a delta function. However, all radiators have finite dimensions, and most experiments\n"
		<< "don't have \"ideal conditions\". Therefore, the 'TammThinTarget_Model' model considers a single finite dimension\n"
		<< "(perpendicular to the charged particle's direction) of a radiator according to Tamm's theoretical approach from\n"
		<< "1939. According to that theory, photons are not emitted in a delta-function thetaChR angle but along\n"
		<< "'thetaChR +- deltaThetaChR'. In this model, according to Tamm's theory, deltaThetaChR is presented with a Gaussian\n"
		<< "distribution, satisfying: FWHM = 2.78 * waveLng / (pi * L * sin(thetaChR)), where L is the thickness of the material.\n"
		<< "See reference: I.E. Tamm \"Radiation emitted by uniformly moving electrons\", J of Phys. USSR 1, (1939) 439-454\n";
}

endChR

#endif // !TammThinTarget_Model_hpp