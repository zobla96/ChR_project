#include "PrimaryGeneratorAction.hpp"

//##########################################
//#######         VERSION 0.3        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

beginChR

//=========public ChR::PrimaryGeneratorAction:: methods=========
PrimaryGeneratorAction* PrimaryGeneratorAction::GetInstance() {
	static thread_local PrimaryGeneratorAction* theInst = new PrimaryGeneratorAction{};
	return theInst;
}

PrimaryGeneratorAction::PrimaryGeneratorAction()
: m_beamSigma(0._um),
m_energy(855._MeV), m_zDistance(0), m_countdown(0),
m_massNo(0), m_atomicNo(0), m_noOfParticles(1u),
m_sigErrGauss{ 0., 0. }, m_sigBeamGauss{ 0., 0. }, m_sinBeamDivergenceTheta(0.) {
	p_theGun = new G4ParticleGun{};
	p_pGeneratorMessenger = new PrimaryGeneratorAction_Messenger{ this };
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
	delete p_theGun;
	delete p_pGeneratorMessenger;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
	double x = 0., y = 0.;
	if (m_beamSigma != 0.) { //by setting m_beamSigma == 0 one can use pencil-like beams
		if (m_countdown % 500 == 0)
			m_sigBeamGauss = std::normal_distribution{ 0., m_beamSigma + m_sigErrGauss(g_mtGen) };
		//the last can break for very low m_beamSigma and high sigma in m_sigErrGauss... no need for safeties for ChR_project's use
		m_countdown++;
		x = m_sigBeamGauss(g_mtGen);
		y = m_sigBeamGauss(g_mtGen);
	}
	if (m_sinBeamDivergenceTheta > 0) {
		double phi = G4UniformRand() * CLHEP::twopi;
		//the correction of the bad CDF from v0.2
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