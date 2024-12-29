//##########################################
//#######        VERSION 1.0.0       #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

/*
ABOUT THE HEADER
----------------

The G4CherenkovProcess class serves as a wrapper class for various
Cherenkov models (their abstract class is G4BaseChR_Model). All
models are stored in a std::vector and are executed based on their
ID in the vector (with a help of the G4ExtraOpticalParameters class).

To better understand how it works, consider running methods DumpInfo
and ProcessDescription while in the G4State_Idle.

The class is loaded through the G4OpticalPhysics_option2 class
*/

#pragma once
#ifndef G4CherenkovProcess_hh
#define G4CherenkovProcess_hh

//G4 headers
#include "G4VDiscreteProcess.hh"
//...
#include "G4BaseChR_Model.hh"
#include "G4ExtraOpticalParameters.hh"
//std:: headers
#include <vector>
#include <typeinfo>

class G4CherenkovProcess_Messenger;

class G4CherenkovProcess : public G4VDiscreteProcess {
	friend G4CherenkovProcess_Messenger;
public:
	G4CherenkovProcess(const G4String& name = "Cherenkov");
	virtual ~G4CherenkovProcess() override;
	G4CherenkovProcess(const G4CherenkovProcess&) = delete;
	G4CherenkovProcess& operator=(const G4CherenkovProcess&) = delete;
	G4CherenkovProcess(G4CherenkovProcess&&) = delete;
	G4CherenkovProcess& operator=(G4CherenkovProcess&&) = delete;

	//=======Overridden inline methods=======
	[[nodiscard]] virtual inline G4double PostStepGetPhysicalInteractionLength(const G4Track&, G4double, G4ForceCondition*) override;
	[[nodiscard]] virtual inline G4VParticleChange* PostStepDoIt(const G4Track&, const G4Step&) override;
	[[nodiscard]] virtual inline G4bool IsApplicable(const G4ParticleDefinition&) override;
	virtual inline void StartTracking(G4Track*) override;
	virtual inline void BuildPhysicsTable(const G4ParticleDefinition&) override;
	virtual inline void PreparePhysicsTable(const G4ParticleDefinition&) override;
	virtual inline G4bool StorePhysicsTable(const G4ParticleDefinition*, const G4String&, G4bool) override;
	virtual inline G4bool RetrievePhysicsTable(const G4ParticleDefinition*, const G4String&, G4bool) override;
	virtual inline void BuildWorkerPhysicsTable(const G4ParticleDefinition&) override;
	virtual inline void PrepareWorkerPhysicsTable(const G4ParticleDefinition&) override;

	//=======Overridden non-inline methods=======
	[[nodiscard]] virtual G4double MinPrimaryEnergy(const G4ParticleDefinition*, const G4Material*) override;
	virtual void DumpInfo() const override;
	virtual void ProcessDescription(std::ostream& outStream = std::cout) const override;
	
	//=======Additional inlines=======
	inline G4bool AddNewChRModel(G4BaseChR_Model* aModel);
	inline const G4BaseChR_Model* GetChRModel(const size_t anID) const;
	inline size_t GetNumberOfRegisteredModels() const;
protected:
	virtual G4double GetMeanFreePath(const G4Track&, G4double, G4ForceCondition*) override { return -1; }; //it was pure virtual
	std::vector<G4BaseChR_Model*> m_registeredModels;
private:
	G4CherenkovMatData* p_selectedMatData = nullptr;
	G4BaseChR_Model* p_selectedModel = nullptr;
	G4CherenkovProcess_Messenger* p_ChRProcessMessenger = nullptr;
};

//=======Inlines=======
G4double G4CherenkovProcess::PostStepGetPhysicalInteractionLength(const G4Track& aTrack, G4double previousStepSize, G4ForceCondition* aForceCondition) {
	try {
		// the following might be a problem if the rebuilding geometry and adding new LV in later phases
		// might need to add 'if (p_selectedMatData) throw ...'
		p_selectedMatData = &G4ExtraOpticalParameters::GetInstance()->FindOrCreateChRMatData(aTrack.GetVolume()->GetLogicalVolume());
		p_selectedModel = m_registeredModels.at(p_selectedMatData->m_executeModel);
		return p_selectedModel->PostStepModelIntLength(aTrack, previousStepSize, aForceCondition);
	}
	catch (std::out_of_range) {
		std::ostringstream err;
		err << "A Cherenkov model with ID: "
			<< G4ExtraOpticalParameters::GetInstance()->FindOrCreateChRMatData(aTrack.GetVolume()->GetLogicalVolume()).m_executeModel
			<< " not found while you are trying to execute it!\n";
		G4Exception("G4CherenkovProcess::PostStepGetPhysicalInteractionLength", "FE_ChRProcess01", FatalException, err);
		return DBL_MAX; //just to make sure no compiler would complain
	}
	catch (...) {
		std::ostringstream err;
		err << "An unexpected error has occurred while trying to obtain Cherenkov model interaction length!\nPlease, report this bug.\n";
		G4Exception("G4CherenkovProcess::PostStepGetPhysicalInteractionLength", "FE_ChRProcess02", FatalException, err);
		return DBL_MAX; //just to make sure no compiler would complain
	}
}

G4VParticleChange* G4CherenkovProcess::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) {
	return p_selectedModel->PostStepModelDoIt(aTrack, aStep, *p_selectedMatData);
}

G4bool G4CherenkovProcess::IsApplicable(const G4ParticleDefinition& aParticle) {
	// copy/paste from G4Cerenkov
	return (aParticle.GetPDGCharge() != 0.0 &&
		aParticle.GetPDGMass() != 0.0 &&
		aParticle.GetParticleName() != "chargedgeantino" &&
		!aParticle.IsShortLived())
		? true : false;
}

void G4CherenkovProcess::StartTracking(G4Track* aTrack) {
	//not sure why I used this function, but keep it here... might want it in the future
	G4VProcess::StartTracking(aTrack);
}

void G4CherenkovProcess::BuildPhysicsTable(const G4ParticleDefinition& aParticle) {
	for (auto* aModel : m_registeredModels)
		aModel->BuildModelPhysicsTable(aParticle);
}

void G4CherenkovProcess::PreparePhysicsTable(const G4ParticleDefinition& aParticle) {
	for (auto* aModel : m_registeredModels)
		aModel->PrepareModelPhysicsTable(aParticle);
}

G4bool G4CherenkovProcess::StorePhysicsTable(const G4ParticleDefinition* aParticle, const G4String& aString, G4bool aFlag) {
	G4bool returnValue = true;
	for (auto* aModel : m_registeredModels)
		if (!aModel->StoreModelPhysicsTable(aParticle, aString, aFlag))
			returnValue = false;
	return returnValue;
}

G4bool G4CherenkovProcess::RetrievePhysicsTable(const G4ParticleDefinition* aParticle, const G4String& aString, G4bool aFlag) {
	G4bool returnValue = true;
	for (auto* aModel : m_registeredModels)
		if (!aModel->RetrieveModelPhysicsTable(aParticle, aString, aFlag))
			returnValue = false;
	return returnValue;
}

void G4CherenkovProcess::BuildWorkerPhysicsTable(const G4ParticleDefinition& aParticle) {
	for (auto* aModel : m_registeredModels)
		aModel->BuildWorkerModelPhysicsTable(aParticle);
}

void G4CherenkovProcess::PrepareWorkerPhysicsTable(const G4ParticleDefinition& aParticle) {
	for (auto* aModel : m_registeredModels)
		aModel->PrepareWorkerModelPhysicsTable(aParticle);
}

//=======Additional inlines=======
G4bool G4CherenkovProcess::AddNewChRModel(G4BaseChR_Model* aModel) {
	for (G4BaseChR_Model* aRegisteredModel : m_registeredModels) {
		if (typeid(*aRegisteredModel) == typeid(*aModel)) {
			std::ostringstream err;
			err << "A Cherenkov model \"" << typeid(*aModel).name() << "\" has already been registered!\n";
			G4Exception("G4CherenkovProcess::AddNewChRModel", "WE_ChRProcess01", JustWarning, err);
			return false;
		}
	}
	m_registeredModels.push_back(aModel);
	return true;
}

const G4BaseChR_Model* G4CherenkovProcess::GetChRModel(const size_t anID) const {
	try {
		return m_registeredModels.at(anID);
	}
	catch (std::out_of_range) {
		std::ostringstream err;
		err << "A Cherenkov model with ID: " << anID << " not found!\n";
		G4Exception("G4CherenkovProcess::GetChRModel", "WE_ChRProcess02", JustWarning, err);
		return nullptr;
	}
	catch (...) {
		std::ostringstream err;
		err << "An unexpected error has occurred while searching for Cherenkov model with ID: " << anID << "! Please, report the bug.\n";
		G4Exception("G4CherenkovProcess::GetChRModel", "FE_ChRProcess03", FatalException, err);
		return nullptr; //just to make sure no compiler would complain
	}
}

size_t G4CherenkovProcess::GetNumberOfRegisteredModels() const {
	return m_registeredModels.size();
}

#endif // !G4CherenkovProcess_hh