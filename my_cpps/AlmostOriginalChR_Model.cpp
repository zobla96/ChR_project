#include "AlmostOriginalChR_Model.hpp"

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

beginChR

//this can be done with verbose as well, but preprocessor removes it from the code while with verbose everything stays
#if 0 //'0' for tracking without Debug
#define AlmostOriginalChR_test
static std::atomic<int> localInstanceCounter = 0; //the idea is to set '#if 1' only for a single thread trying, but still... atomic
#endif

//=========public ChR::AlmostOriginalChR_Model:: methods=========
AlmostOriginalChR_Model::AlmostOriginalChR_Model(const char* name)
: BaseChR_Model(name) {

#ifdef AlmostOriginalChR_test
	std::cout.fill('=');
	std::cout << std::setw(51) << '\n'
		<< "Constructor AlmostOriginalChR_Model::AlmostOriginalChR_Model(...) #" << ++localInstanceCounter << '\n'
		<< std::setw(51) << '\n';
#endif // AlmostOriginalChR_test

}

AlmostOriginalChR_Model::~AlmostOriginalChR_Model() {

#ifdef AlmostOriginalChR_test
	std::cout.fill('=');
	std::cout << std::setw(51) << '\n'
		<< "Destructor AlmostOriginalChR_Model::AlmostOriginalChR_Model(...) #" << localInstanceCounter-- << '\n'
		<< std::setw(51) << '\n';
#endif // AlmostOriginalChR_test

}

G4VParticleChange* AlmostOriginalChR_Model::PostStepModelDoIt(const G4Track& aTrack, const G4Step& aStep) {

#ifdef AlmostOriginalChR_test
	std::cout.fill('=');
	std::cout.precision(4); //floating point precision
	std::cout << std::setw(75) << '\n';
	std::cout.fill(' '); //ASCII space -> 32
	std::cout << "Begin of AlmostOriginalChR_Model::PostStepModelDoIt for particle:\n" << std::setw(20) << std::right
		<< aTrack.GetParticleDefinition()->GetParticleName() << " of kinetic energy: " << aStep.GetPreStepPoint()->GetKineticEnergy() / MeV << " MeV\n";
#endif // AlmostOriginalChR_test

	p_particleChange->Initialize(aTrack);
	const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
	const G4Material* aMaterial = aTrack.GetMaterial();

#ifdef AlmostOriginalChR_test
	std::cout << "Current material: " << std::left << std::setw(20) << aMaterial->GetName() << " Material index: " << aMaterial->GetIndex() << '\n';
#endif // AlmostOriginalChR_test

	if (m_ChRPhysDataVec[aMaterial->GetIndex()].betaVector.size() <= 1) {

#ifdef AlmostOriginalChR_test
		std::cout << "End of AlmostOriginalChR_Model::PostStepModelDoIt(...)\n";
		std::cout.fill('=');
		std::cout << std::setw(51) << '\n';
#endif // AlmostOriginalChR_test

		return p_particleChange;
	}

	const G4StepPoint* preStepPoint = aStep.GetPreStepPoint();
	const G4StepPoint* postStepPoint = aStep.GetPostStepPoint();

	const G4ThreeVector x0 = preStepPoint->GetPosition();
	const G4ThreeVector p0 = aStep.GetDeltaPosition().unit();
	const double t0 = preStepPoint->GetGlobalTime();

	G4PhysicsFreeVector* RIndex = aMaterial->GetMaterialPropertiesTable()->GetProperty(kRINDEX);
	if(!RIndex)
		RIndex = aMaterial->GetMaterialPropertiesTable()->GetProperty(kREALRINDEX);

	const double charge = aParticle->GetDefinition()->GetPDGCharge();
	const double beta = (preStepPoint->GetBeta() + postStepPoint->GetBeta()) * 0.5;

	double meanNumberOfPhotons = CalculateAverageNumberOfPhotons(charge, beta, aMaterial);

	if (meanNumberOfPhotons <= 0.0) {//unchanged particle; number of secondaries is 0 anyway after p_particleChange->Initialize(aTrack);

#ifdef AlmostOriginalChR_test
		std::cout << "End of AlmostOriginalChR_Model::PostStepModelDoIt(...)\n";
		std::cout.fill('=');
		std::cout << std::setw(51) << '\n';
#endif // AlmostOriginalChR_test

		return p_particleChange;
	}

	meanNumberOfPhotons = meanNumberOfPhotons * aStep.GetStepLength();
	m_noOfPhotons = (int)G4Poisson(meanNumberOfPhotons);

#ifdef AlmostOriginalChR_test
	std::cout << "Number of photons: " << m_noOfPhotons << '\n';
#endif // AlmostOriginalChR_test

	//got no idea what's the point of 'p_G4OptParameters->GetCerenkovStackPhotons()', but staying consistent...
	if (m_noOfPhotons <= 0 || !p_G4OptParameters->GetCerenkovStackPhotons()) {

#ifdef AlmostOriginalChR_test
		std::cout << "End of AlmostOriginalChR_Model::PostStepModelDoIt(...)\n";
		std::cout.fill('=');
		std::cout << std::setw(51) << '\n';
#endif // AlmostOriginalChR_test

		return p_particleChange;
	}
	p_particleChange->SetNumberOfSecondaries(m_noOfPhotons);

	if (p_G4OptParameters->GetCerenkovTrackSecondariesFirst()) {
		if (aTrack.GetTrackStatus() == fAlive)
			p_particleChange->ProposeTrackStatus(fSuspend);
	}

	double lossEnergy = 0.;

#ifdef AlmostOriginalChR_test
	std::cout.fill('_');
	std::cout << std::right << std::setw(75) << '\n'
		<< "| Photon number |  Energy [eV]  |  RIndex  |  Phi [deg]  |  Theta [deg]  |\n"
		<< '|' << std::setw(16) << '|' << std::setw(16) << '|' << std::setw(11) << '|'
		<< std::setw(14) << '|' << std::setw(17) << "|\n";
	std::cout.fill(' ');
	std::cout.precision(2);
#endif // AlmostOriginalChR_test

	for (int i = 0; i < m_noOfPhotons; ++i) {
		// m_Emin and m_Emax are used to speed up this do-while loop. In general, in it's original form,
		// the sole point of the loop is to prevent generating photons with sin2Theta < 0.
		// therefore, the condition in the while is set to such
		// Determine photon energy
		double rand;
		double sampledEnergy, sampledRI;
		double cosTheta, sin2Theta;
		// sample an energy
		do {
			rand = G4UniformRand();
			sampledEnergy = m_Emin + rand * (m_Emax - m_Emin);
			sampledRI = RIndex->Value(sampledEnergy);
			cosTheta = 1. / (sampledRI * beta); //might give > 1. for strange n(E) functions
			sin2Theta = (1.0 - cosTheta) * (1.0 + cosTheta);
		} while (sin2Theta <= 0.);
		// Create photon momentum direction vector. The momentum direction is still
		// with respect to the coordinate system where the primary particle
		// direction is aligned with the z axis
		rand = G4UniformRand();
		double phi = CLHEP::twopi * rand;
		double sinPhi = std::sin(phi);
		double cosPhi = std::cos(phi);
		double sinTheta = std::sqrt(sin2Theta);

#ifdef AlmostOriginalChR_test
		std::string photNo = "#" + std::to_string(i + 1);
		std::cout << '|' << std::setw(9) << photNo << std::setw(7) << '|'
			<< std::fixed << std::setw(10) << sampledEnergy / eV << std::setw(6) << '|'
			<< std::setprecision(4) << std::setw(8) << RIndex->Value(sampledEnergy) << std::setw(3) << '|'
			<< std::setprecision(2) << std::setw(9) << phi / deg << std::setw(5) << '|'
			<< std::setw(10) << std::acos(cosTheta) / deg << std::setw(7) << "|\n";
#endif // AlmostOriginalChR_test

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

		//The following seems like a very expensive way to change rand distribution
		//However, the idea might be correct for very low energies, i.e.,
		//more photons are emitted from around preStep than around postStep.
		//Any suggestions?
		//=================================================================
		//double beta1 = preStepPoint->GetBeta();
		//double beta2 = postStepPoint->GetBeta();
		//double meanNumberOfPhotons1 = CalculateAverageNumberOfPhotons(charge, beta1, aMaterial);
		//double meanNumberOfPhotons2 = CalculateAverageNumberOfPhotons(charge, beta2, aMaterial);
		//double numberOfPhotons, N;
		//do {
		//	rand = G4UniformRand();
		//	numberOfPhotons = meanNumberOfPhotons1 - rand * (meanNumberOfPhotons1 - meanNumberOfPhotons2);
		//	N = G4UniformRand() * std::max(meanNumberOfPhotons1, meanNumberOfPhotons2);
		//	// Loop checking, 07-Aug-2015, Vladimir Ivanchenko
		//} while (N > numberOfPhotons);
		//=================================================================
		rand = G4UniformRand(); //More or less, this line can replace the commented part... however, the distribution remains uniform

		double delta = rand * aStep.GetStepLength();
		double deltaTime = delta / (preStepPoint->GetVelocity() + rand * (postStepPoint->GetVelocity() - preStepPoint->GetVelocity()) *	0.5);

		double aSecondaryTime = t0 + deltaTime;
		G4ThreeVector aSecondaryPosition = x0 + rand * aStep.GetDeltaPosition();

		// Generate new G4Track object:
		G4Track* aSecondaryTrack = new G4Track(aCerenkovPhoton, aSecondaryTime, aSecondaryPosition);

		aSecondaryTrack->SetTouchableHandle(aStep.GetPreStepPoint()->GetTouchableHandle());
		aSecondaryTrack->SetParentID(aTrack.GetTrackID());
		p_particleChange->AddSecondary(aSecondaryTrack);
	}

#ifdef AlmostOriginalChR_test
	std::cout.fill('_');
	std::cout << '|' << std::setw(16) << '|' << std::setw(16) << '|' << std::setw(11) << '|' << std::setw(14) << '|' << std::setw(17) << "|\n";
#endif // AlmostOriginalChR_test

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

#ifdef AlmostOriginalChR_test
	std::cout << "Emitted energy: " << lossEnergy / eV << " eV\nEnd of AlmostOriginalChR_Model::PostStepModelDoIt(...)\n";
	std::cout.fill('=');
	std::cout << std::setw(75) << '\n';
#endif // AlmostOriginalChR_test

	return p_particleChange;
}

void AlmostOriginalChR_Model::DumpModelInfo() const {
	std::cout << "\"AlmostOriginalChR_Model\" is a standard Cherenkov radiation model that should be used for most Cherenkov\n"
		<< "radiation simulations. The model is very similar to the original G4Cerenkov class and some parts of the\n"
		<< "code have even been copied! Still, the original class considered that energy and refractive index\n"
		<< "are sorted vectors. Due to that, inconvenient results might be obtained in some cases for critical energies.\n"
		<< "In this model, the physics tables are constructed more conveniently to treat any refractive-index\n"
		<< "behavior. Also, some slight changes might be noticed in the code to reduce the possible impact of some loops\n"
		<< "on the number of processor cycles.\n";
}

endChR