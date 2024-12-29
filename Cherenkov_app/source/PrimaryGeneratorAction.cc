//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

// user headers
#include "PrimaryGeneratorAction.hh"
// G4 headers
#include "G4SystemOfUnits.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "Randomize.hh"

beginChR

//=========public ChR::PrimaryGeneratorAction:: methods=========

PrimaryGeneratorAction::PrimaryGeneratorAction()
: m_beamSigma(0. * um), m_sinBeamDivergenceTheta(0.), m_zDistance(0.) {
	// experimental value: m_beamSigma = 536. * um
	p_theGun = new G4ParticleGun{};
	p_theGun->SetNumberOfParticles(1);
	p_theGun->SetParticleDefinition(G4ParticleTable::GetParticleTable()->FindParticle("e-"));
	p_theGun->SetParticleEnergy(855. * MeV); // the experimental value
	p_pGeneratorMessenger = new PrimaryGeneratorAction_Messenger{ this };
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
	delete p_theGun;
	delete p_pGeneratorMessenger;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
	G4double x = 0., y = 0.;
	if (m_beamSigma > 0.) { //by setting m_beamSigma == 0 one can use pencil-like beams
		x = G4RandGauss::shoot(0., m_beamSigma);
		y = G4RandGauss::shoot(0., m_beamSigma);
	}
	if (m_sinBeamDivergenceTheta > 0.) {
		G4double phi = G4UniformRand() * CLHEP::twopi;
		G4double sinTheta;
		do {
			sinTheta = m_sinBeamDivergenceTheta * std::sqrt(-2. * std::log(G4UniformRand()));
		} while (sinTheta > 1.); //this should never happen for some reasonable divergence angles, but just in case!
		G4double cosTheta = std::sqrt(1. - sinTheta * sinTheta);
		p_theGun->SetParticleMomentumDirection(G4ThreeVector{ sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta });
	}
	else
		p_theGun->SetParticleMomentumDirection(G4ThreeVector{ 0., 0., 1. });
	p_theGun->SetParticlePosition(G4ThreeVector{ x, y, -m_zDistance });
	p_theGun->GeneratePrimaryVertex(anEvent);
}

endChR