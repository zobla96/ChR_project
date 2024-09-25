//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#include "PrimaryGeneratorAction.hpp"

beginChR

//=========public ChR::PrimaryGeneratorAction:: methods=========

PrimaryGeneratorAction* PrimaryGeneratorAction::GetInstance() {
	static thread_local PrimaryGeneratorAction* theInst = new PrimaryGeneratorAction{};
	return theInst;
}

PrimaryGeneratorAction::PrimaryGeneratorAction()
: m_beamSigma(0._um), m_sinBeamDivergenceTheta(0.), m_energy(855._MeV),
m_zDistance(0.), m_noOfParticles(1u), m_massNo(0), m_atomicNo(0) {
	p_theGun = new G4ParticleGun{};
	p_pGeneratorMessenger = new PrimaryGeneratorAction_Messenger{ this };
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
	delete p_theGun;
	delete p_pGeneratorMessenger;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
	double x = 0., y = 0.;
	if (m_beamSigma > 0.) { //by setting m_beamSigma == 0 one can use pencil-like beams
		x = G4RandGauss::shoot(0., m_beamSigma);
		y = G4RandGauss::shoot(0., m_beamSigma);
	}
	if (m_sinBeamDivergenceTheta > 0.) {
		double phi = G4UniformRand() * CLHEP::twopi;
		double sinTheta;
		do {
			sinTheta = m_sinBeamDivergenceTheta * std::sqrt(-2. * std::log(G4UniformRand()));
		} while (sinTheta > 1.); //this should never happen for some reasonable divergence angles, but just in case!
		double cosTheta = std::sqrt(1. - sinTheta * sinTheta);
		//sin or cos phi is not important, it's 2*pi anyway
		p_theGun->SetParticleMomentumDirection(G4ThreeVector{ sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta });
	}
	else
		p_theGun->SetParticleMomentumDirection(G4ThreeVector{ 0., 0., 1. });
	p_theGun->SetParticlePosition(G4ThreeVector{ x, y, -m_zDistance });
	p_theGun->GeneratePrimaryVertex(anEvent);
}

endChR