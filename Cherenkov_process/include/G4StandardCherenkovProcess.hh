//##########################################
//#######        VERSION 1.1.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

/*
ABOUT THE HEADER
----------------

The process represents the most commonly used Cherenkov radiation, i.e.,
it is based on the standard Frank-Tamm Cherenkov radiation theory. That
means the model can be used for "ideal" (infinite) radiators. The process
is very similar to the G4Cerenkov process, while the physics tables are
slightly improved and used differently.
Read the DumpInfo and ProcessDescription methods for more information
about the process
The process is loaded through the G4OpticalPhysics_option1 class
*/

#ifndef G4StandardCherenkovProcess_hh
#define G4StandardCherenkovProcess_hh

//G4 headers
#include "G4VDiscreteProcess.hh"
//...
#include "G4ChRPhysicsTableData.hh"

class G4StandardChRProcess_Messenger;

class G4StandardCherenkovProcess : public G4VDiscreteProcess
{
  friend G4StandardChRProcess_Messenger;
public:
  using G4ChRPhysicsTableVector = std::vector<G4ChRPhysTableData>;

  G4StandardCherenkovProcess(const G4String& name = "StandardCherenkov");
  virtual ~G4StandardCherenkovProcess() override;
  G4StandardCherenkovProcess(const G4StandardCherenkovProcess&) = delete;
  G4StandardCherenkovProcess& operator=(const G4StandardCherenkovProcess&) = delete;
  G4StandardCherenkovProcess(G4StandardCherenkovProcess&&) = delete;
  G4StandardCherenkovProcess& operator=(G4StandardCherenkovProcess&&) = delete;

  [[nodiscard]] virtual inline G4bool IsApplicable(const G4ParticleDefinition&) override;

  [[nodiscard]] virtual G4double PostStepGetPhysicalInteractionLength(const G4Track&, G4double, G4ForceCondition*) override;
  [[nodiscard]] virtual G4VParticleChange* PostStepDoIt(const G4Track&, const G4Step&) override;
  [[nodiscard]] virtual G4double MinPrimaryEnergy(const G4ParticleDefinition*, const G4Material*) override;
  virtual void BuildPhysicsTable(const G4ParticleDefinition&) override;
  virtual void DumpInfo() const override;
  virtual void ProcessDescription(std::ostream& outStream = std::cout) const override;

  static void PrintChRPhysDataVec(const unsigned char printLevel = 0, const G4Material* aMaterial = nullptr);
  // aMaterial == nullptr -> prints physics tables for all registered materials
  // aMaterial == someMaterial -> prints physics tables for a someMaterial
  // printLevel == 0 -> print only basic available information about registered physics tables
  // printLevel == 1 -> print standard + p_valuesCDF values
  // printLevel >= 2 -> print all available information about registered physics tables

  //=======Set inlines=======
  inline static unsigned int SetNoOfBetaSteps(const unsigned int);
  inline void SetUseEnergyLoss(const G4bool);
  //=======Get inlines=======
  [[nodiscard]] inline static unsigned int GetNoOfBetaSteps();
  [[nodiscard]] inline G4bool GetUseEnergyLoss() const;
protected:
  [[nodiscard]] virtual G4double CalculateAverageNumberOfPhotons(const G4double aCharge, const G4double betaValue, const size_t materialID);
  virtual G4double GetMeanFreePath(const G4Track&, G4double, G4ForceCondition*) override { return -1; }; //it was pure virtual
private:
  static G4ChRPhysicsTableVector fChRPhysDataVec;
  static unsigned int fNoOfBetaSteps;
  //=======Member variables=======
  G4ParticleChange* fParticleChange;
  G4StandardChRProcess_Messenger* fChRProcessMessenger;
  G4bool fUseEnergyLoss;
  // 7 wasted bytes...
  //==============================
  static G4bool AddExoticRIndexPhysicsTable(const size_t materialID, G4bool forceExoticFlag = false);
  static void RemoveExoticRIndexPhysicsTable(const size_t materialID);
};

//=======Set inlines=======

unsigned int G4StandardCherenkovProcess::SetNoOfBetaSteps(const unsigned int value)
{
  unsigned int temp = fNoOfBetaSteps;
  fNoOfBetaSteps = value;
  return temp;
}

void G4StandardCherenkovProcess::SetUseEnergyLoss(const G4bool value)
{
  fUseEnergyLoss = value;
}

//=======Get inlines=======

unsigned int G4StandardCherenkovProcess::GetNoOfBetaSteps()
{
  return fNoOfBetaSteps;
}

G4bool G4StandardCherenkovProcess::GetUseEnergyLoss() const
{
  return fUseEnergyLoss;
}

//=======Additional inlines=======

G4bool G4StandardCherenkovProcess::IsApplicable(const G4ParticleDefinition& aParticle)
{
  // copy/paste from G4Cerenkov
  return (aParticle.GetPDGCharge() != 0.0 &&
    aParticle.GetPDGMass() != 0.0 &&
    aParticle.GetParticleName() != "chargedgeantino" &&
    !aParticle.IsShortLived())
    ? true : false;
}

#endif // !G4StandardCherenkovProcess_hh