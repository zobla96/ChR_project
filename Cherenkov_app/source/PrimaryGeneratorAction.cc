//##########################################
//#######        VERSION 1.1.0       #######
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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//=========public ChR::PrimaryGeneratorAction:: methods=========
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::PrimaryGeneratorAction()
: fTheGun(new G4ParticleGun{}),
  fPGeneratorMessenger(new PrimaryGeneratorAction_Messenger{ this }),
  fBeamSigma(0. * um),
  fSinBeamDivergenceTheta(0.),
  fZDistance(0.)
{

  // experimental value: fBeamSigma = 536. * um
  fTheGun->SetNumberOfParticles(1);
  fTheGun->SetParticleDefinition(G4ParticleTable::GetParticleTable()->FindParticle("e-"));
  fTheGun->SetParticleEnergy(855. * MeV); // the experimental value
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fTheGun;
  delete fPGeneratorMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  G4double x = 0., y = 0.;
  if (fBeamSigma > 0.) { //by setting fBeamSigma == 0 one can use pencil-like beams
    x = G4RandGauss::shoot(0., fBeamSigma);
    y = G4RandGauss::shoot(0., fBeamSigma);
  }
  if (fSinBeamDivergenceTheta > 0.) {
    G4double phi = G4UniformRand() * CLHEP::twopi;
    G4double sinTheta;
    do {
      sinTheta = fSinBeamDivergenceTheta * std::sqrt(-2. * std::log(G4UniformRand()));
    } while (sinTheta > 1.); //this should never happen for some reasonable divergence angles, but just in case!
    G4double cosTheta = std::sqrt(1. - sinTheta * sinTheta);
    fTheGun->SetParticleMomentumDirection(G4ThreeVector{ sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta });
  }
  else
    fTheGun->SetParticleMomentumDirection(G4ThreeVector{ 0., 0., 1. });
  fTheGun->SetParticlePosition(G4ThreeVector{ x, y, -fZDistance });
  fTheGun->GeneratePrimaryVertex(anEvent);
}

endChR