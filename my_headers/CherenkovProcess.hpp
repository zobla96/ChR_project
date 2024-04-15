#pragma once
#ifndef CherenkovProcess_hpp
#define CherenkovProcess_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//User built headers
#include "AlmostOriginalChR_Model.hpp"
#include "TammThinTarget_Model.hpp"
#include "thePCM_Model.hpp"
#include "AccessPhysicsVectors.hpp"
#include "MyOpticalParameters.hpp"
#include "CherenkovProcess_Messenger.hpp"
//G4 headers
#include "G4VDiscreteProcess.hh"
//...
#include "G4GeometryTolerance.hh"
#include "G4ProcessManager.hh"
//std:: headers
#include <iostream>
#include <map>
#include <type_traits>

beginChR

template <typename T>
class CherenkovProcess_Messenger;

//don't inherite G4Cerenkov, it's not an inheritance-safe class! Moreover, it has no virtuals and protected...
//Making the following process as a template so anyone can add new models and just spcialize the template process
template<typename T>
class CherenkovProcess : public G4VDiscreteProcess {
public:
	static_assert(std::is_enum_v<T> == true, "The class CherenkovProcess<T> is designed to support enum tpye of T!");
	explicit CherenkovProcess(std::string name = "Cherenkov");
	virtual ~CherenkovProcess() override;
	CherenkovProcess(const CherenkovProcess&) = delete;
	CherenkovProcess& operator=(const CherenkovProcess&) = delete;
	CherenkovProcess(CherenkovProcess&&) = delete;
	CherenkovProcess& operator=(CherenkovProcess&&) = delete;
	_NODISCARD virtual double PostStepGetPhysicalInteractionLength(const G4Track&, double, G4ForceCondition*) override;
	_NODISCARD virtual G4VParticleChange* PostStepDoIt(const G4Track&, const G4Step&) override;
	_NODISCARD virtual double MinPrimaryEnergy(const G4ParticleDefinition*, const G4Material*) override;
	virtual inline bool IsApplicable(const G4ParticleDefinition&) override;
	virtual void StartTracking(G4Track*) override;
	virtual void BuildPhysicsTable(const G4ParticleDefinition&) override;
	virtual void PreparePhysicsTable(const G4ParticleDefinition&) override;
	virtual bool StorePhysicsTable(const G4ParticleDefinition*, const G4String&, bool) override;
	_NODISCARD virtual bool RetrievePhysicsTable(const G4ParticleDefinition*, const G4String&, bool) override;
	virtual void DumpInfo() const override;
	virtual void ProcessDescription(std::ostream& outStream = std::cout) const override;
	virtual void BuildWorkerPhysicsTable(const G4ParticleDefinition&) override;
	virtual void PrepareWorkerPhysicsTable(const G4ParticleDefinition&) override;
	//=======Methods around m_registeredModels=======
	inline bool AddNewChRModel(const T, BaseChR_Model*);
	inline bool AddNewChRModel(const std::pair<T, BaseChR_Model*>);
	_NODISCARD inline BaseChR_Model* CheckIfAModelIsRegistered(const T) const;
	_NODISCARD inline const std::map<T, BaseChR_Model*>& GetChRRegisteredModels() const; //obtaining this member is user's responsibility!
	//=======Set inlines=======
	inline bool ClearFlag();
	//=======Get inlines=======
	_NODISCARD inline bool GetFlag() const;
protected:
	_NODISCARD virtual double GetMeanFreePath(const G4Track&, double, G4ForceCondition*) override; //pure virtual
	std::map<T, BaseChR_Model*> m_registeredModels;
private:
	BaseChR_Model* m_selectedModel = nullptr;
	bool m_registered1DThicknessModel;
	bool m_processFlag;
	/*while flag == true, the process CherenkovProcess<T> will be executed... if there's a
	problem it will be stoped untill the user clears the bad-state flag. For this project,
	it's a redundant bool as I'll specialize (see at the end of the .hpp) the template class*/
	CherenkovProcess_Messenger<T>* p_ChRProcessMessenger = nullptr;
};

//=======Methods around m_registeredModels for CherenkovProcess<T>=======
template<typename T>
bool CherenkovProcess<T>::AddNewChRModel(const T key, BaseChR_Model* val) {
	if (!m_processFlag) m_processFlag = true;
	if (!m_registered1DThicknessModel && val->GetFiniteThicknessCondition())
		m_registered1DThicknessModel = true;
	return m_registeredModels.insert(std::make_pair(key, val)).second;
}
template<typename T>
inline bool CherenkovProcess<T>::AddNewChRModel(const std::pair<T, BaseChR_Model*> aPair) {
	if (!m_processFlag) m_processFlag = true;
	if (!m_registered1DThicknessModel && aPair.second->GetFiniteThicknessCondition())
		m_registered1DThicknessModel = true;
	return m_registeredModels.insert(aPair).second;
}
template<typename T>
_NODISCARD inline BaseChR_Model* CherenkovProcess<T>::CheckIfAModelIsRegistered(const T key) const {
	try {
		return m_registeredModels.at(key);
	}
	catch (std::out_of_range) {
		const char* location = "ChR::CherenkovProcess<T>::CheckIfAModelIsRegistered(...)";
		std::ostringstream reason;
		reason << "No model with the key " << std::quoted(typeid(key).name()) << " was not loaded into m_registeredModels\n";
		G4Exception(location, "WE1000", JustWarning, reason);
		return nullptr;
	}
	catch (...) {
		G4Exception("ChR::CherenkovProcess<T>::CheckIfAModelIsRegistered(...)", "FE1000", FatalException, "This one was not expected!\n");
		return nullptr; //silence comiler
	}
}
template<typename T>
_NODISCARD inline const std::map<T, BaseChR_Model*>& CherenkovProcess<T>::GetChRRegisteredModels() const {
	return m_registeredModels;
}

//=======Inlines for CherenkovProcess<T>=======
template<typename T>
bool CherenkovProcess<T>::IsApplicable(const G4ParticleDefinition& aParticle) { //copy/paste from G4Cerenkov
	return (aParticle.GetPDGCharge() != 0.0 &&
		aParticle.GetPDGMass() != 0.0 &&
		aParticle.GetParticleName() != "chargedgeantino" &&
		!aParticle.IsShortLived())
		? true : false;
}

//=======Set inlines for CherenkovProcess<T>=======
template<typename T>
bool CherenkovProcess<T>::ClearFlag() {
	if (m_processFlag == false) {
		m_processFlag = true;
		if (verboseLevel > 0)
			std::cout << "Cherenkov process flag cleared!\n";
		return false;
	}
	return true;
}

//=======Get inlines for CherenkovProcess<T>=======
template <typename T>
bool CherenkovProcess<T>::GetFlag() const {
	return m_processFlag;
}

//=========public ChR::CherenkovProcess<T>:: methods=========
template<typename T>
CherenkovProcess<T>::CherenkovProcess(std::string name)
:G4VDiscreteProcess(name, fElectromagnetic),
m_processFlag(false),
m_registered1DThicknessModel(false) {
	p_ChRProcessMessenger = new CherenkovProcess_Messenger<T>{ this };
	// a user process - I think no subtype
	if (AddNewChRModel(T::Default, new AlmostOriginalChR_Model{}) && verboseLevel > 0)
		std::cout << "AlmostOriginalChR_Model has been added to CherenkovProcess\n";
}

template<typename T>
CherenkovProcess<T>::~CherenkovProcess() {
	delete p_ChRProcessMessenger;
	for (auto& [key, model] : m_registeredModels)
		delete model; //not using smart_pointers
}

template<typename T>
double CherenkovProcess<T>::PostStepGetPhysicalInteractionLength(const G4Track& aTrack, double previousStepSize, G4ForceCondition* aCondition) {
	*aCondition = NotForced;
	if (!m_processFlag) {
		if (verboseLevel > 1)
			G4Exception("ChR::CherenkovProcess<T>::PostStepDoIt", "WE1001", JustWarning, "Bad state flag for Cherenkov process!\n");
		return DBL_MAX;
	}
	m_selectedModel = nullptr;
	//now figuring what model to use... it would be the best to speicalize this method, but I'll make it as usable as possible anyway.
	//Of course, that refers to those creating some additional models which are not included in this project
	//(then template classes (parameters) should be modified to include some other enums as well)
	auto myParameters = MyOpticalParameters<T>::GetInstance();
	G4LogicalVolume* currentLV = aTrack.GetVolume()->GetLogicalVolume();
	CherenkovMatData<T>* matData = myParameters->FindChRMatData(currentLV);
	if (matData->m_maxRIndex <= 1.) return DBL_MAX;
	if (matData->m_forceModel != T::Default) {
		BaseChR_Model* model = CheckIfAModelIsRegistered(matData->m_forceModel);
		if (!model) {
			std::ostringstream theError;
			theError << "You are trying to use an unregistered model: " << typeid(matData->m_forceModel).name(); //proveri sta pise od enuma!!!
			G4Exception("CherenkovProcess<T>::PostStepGetPhysicalInteractionLength", "FE1001", FatalException, theError);
		}
		m_selectedModel = model;
		return m_selectedModel->PostStepModelIntLength(aTrack, previousStepSize,  aCondition);
	}
	if (m_registered1DThicknessModel && myParameters->GetForcedThinTargetCondition()
		|| m_registered1DThicknessModel
		&& matData->m_matThickness > G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()
		&& matData->m_matThickness < myParameters->GetThinTargetLimit()) {
		for (auto& [key, model] : m_registeredModels)
			if (model->GetFiniteThicknessCondition()) {
				m_selectedModel = model;
				return m_selectedModel->PostStepModelIntLength(aTrack, previousStepSize, aCondition);
			}
	}
	for (auto& [key, model] : m_registeredModels)
		if (!model->GetFiniteThicknessCondition()) {
			m_selectedModel = model;
			return m_selectedModel->PostStepModelIntLength(aTrack, previousStepSize, aCondition);
		}
	return DBL_MAX;
}

template<typename T>
G4VParticleChange* CherenkovProcess<T>::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) {
	if (!m_processFlag) {
		G4Exception("ChR::CherenkovProcess<T>::PostStepDoIt", "FE1002", FatalException, "Bad state flag for Cherenkov process!\n");
	}
	return m_selectedModel->PostStepModelDoIt(aTrack, aStep);
}

template<typename T>
void CherenkovProcess<T>::StartTracking(G4Track* aTrack) {
	G4VProcess::StartTracking(aTrack);
	if (m_registeredModels.size() == 0 && m_processFlag) {
		m_processFlag = false; //in debug haven't noticed a better way to stop a process after running BeamOn()
		if (verboseLevel > 0)
			G4Exception("CherenkovProcess<T>::StartTracking(...)", "WE1002", JustWarning, "You didn't register any Cherenkov model... the process is deactivated...\n");
	}
}

template<typename T>
double CherenkovProcess<T>::MinPrimaryEnergy(const G4ParticleDefinition* aParticle, const G4Material* aMaterial) {
	double restE = aParticle->GetPDGMass();
	//now a bit of casting x_x
	//be very careful with reinterpret_cast... in this case it's fine to do this according to the inheritance memory model
	AccessPhysicsVector* accessor = reinterpret_cast<AccessPhysicsVector*>(aMaterial->GetMaterialPropertiesTable()->GetProperty(kRINDEX));
	if (!accessor)
		accessor = reinterpret_cast<AccessPhysicsVector*>(aMaterial->GetMaterialPropertiesTable()->GetProperty(kREALRINDEX));
	if (!accessor) {
		std::ostringstream err;
		err << "Material " << std::quoted(aMaterial->GetName()) << " has no registered RIndex property, i.e., no min energy!\n";
		G4Exception("CherenkovProcess<T>::MinPrimaryEnergy(...)", "WE1003", JustWarning, err);
		return 0.;
	}
	double maxN = accessor->GetRealDataVectorMax();
	double value = std::sqrt(restE * restE / (1 + 1 / (maxN * maxN)));
	if (verboseLevel > 0)
		std::cout << "For particle " << std::quoted(aParticle->GetParticleName()) << ", minimum energy required\n"
		<< "for producing Cherenkov photons in material " << std::quoted(aMaterial->GetName()) << " is " << value / MeV << " MeV\n";
	return value;
}

template<typename T>
void CherenkovProcess<T>::BuildPhysicsTable(const G4ParticleDefinition& aParticle) {
	for (auto& [key, model] : m_registeredModels)
		model->BuildModelPhysicsTable(aParticle);
}

template<typename T>
void CherenkovProcess<T>::PreparePhysicsTable(const G4ParticleDefinition& aParticle) {
	MyOpticalParameters<T>::GetInstance()->ScanAndAddUnregisteredLV();
	for (auto& [key, model] : m_registeredModels)
		model->PrepareModelPhysicsTable(aParticle);
}

template<typename T>
bool CherenkovProcess<T>::StorePhysicsTable(const G4ParticleDefinition* aParticle, const G4String& aString, bool aFlag) {
	bool returnValue = true;
	for (auto& [key, model] : m_registeredModels)
		if (!model->StoreModelPhysicsTable(aParticle, aString, aFlag)) returnValue = false;
	return returnValue;
}

template<typename T>
bool CherenkovProcess<T>::RetrievePhysicsTable(const G4ParticleDefinition* aParticle, const G4String& aString, bool aFlag) {
	bool returnValue = true;
	for (auto& [key, model] : m_registeredModels)
		if (!model->StoreModelPhysicsTable(aParticle, aString, aFlag)) returnValue = false;
	return returnValue;
}

template<typename T>
void CherenkovProcess<T>::DumpInfo() const {
	std::cout.fill('=');
	std::cout << std::setw(116) << '\n';
	std::cout << "Begin of CherenkovProcess<T>::DumpInfo():\n\n"
		<< "Template class CherenkovProcess<T> is made as a wrapper for various Cherenkov radiation models.\n"
		<< "All Cherenkov models are stored in a std::map<T, BaseChR_Model*>. Depending on user-loaded data,\n"
		<< "the class manages what Cherenkov models will be executed during the runtime.\n"
		<< "Currently loaded Cherenkov models are:\n";
	size_t temp = 0;
	for (auto& [key, model] : m_registeredModels) {
		std::cout.fill('+');
		std::cout << std::setw(21) << '\n';
		std::cout << "Model #" << ++temp << ": " << typeid(model).name() << '\n';
		model->DumpModelInfo();
	}
	if (temp == 0) {
		std::cout << "You still haven't loaded any models! Without any process registered, you won't be able to use Cherenkov radiation in simulations!\n";
	}
	std::cout << std::setw(21) << '\n';
	std::cout	<< "If you want to read about what Cherenkov process is, use \"CherenkovProcess<T>::ProcessDescription()\"\n"
				<< "End of CherenkovProcess<T>::DumpInfo()\n";
	std::cout.fill('=');
	std::cout << std::setw(116) << '\n';
	std::cout << std::endl;
}

template<typename T>
void CherenkovProcess<T>::ProcessDescription(std::ostream& outStream) const {
	outStream.fill('=');
	outStream << std::setw(116) << '\n';
	outStream	<< "Begin of CherenkovProcess<T>::ProcessDescription():\n\n"
				<< "The Cherenkov radiation was discovered in 1934 by S.I. Vavilov and P.A. Cherenkov (Vavilov's student).\n"
				<< "The first theoretical explanation was provided in 1937 by I.M. Frank and I.E. Tamm.\n"
				<< "According to our understanding, when a charged particle moves through a medium faster than the phase\n"
				<< "velocity of light, photons of a wide energy range (predominantly in the optical region) are emitted.\n"
				<< "\"Moves faster\" means the condition \"beta * n >= 1\", where \"beta\" is a relativistic reduced velocity (= v / c)\n"
				<< "and n is the refractive index, is satisfied. In order to use this class in Geant4, one must define refractive\n"
				<< "index of a material through the class G4MaterialPropertiesTable. To do so, one needs to provide\n"
				<< "a corresponding key index that can be found in \"G4MaterialPropertiesIndex.hh\" (just remove \"k\" from the enum).\n"
				<< "Also, note that it's advisable to always define absorption as well; otherwise an optical photon might enter\n"
				<< "an infinite loop of total internal reflections.\n\n"
				<< "To see details about this specific C++ class, use CherenkovProcess<T>::DumpInfo() method\n\n"
				<< "Some English literature I know of that one might find interesting and is a nice summary of everything:\n"
				<< "1. J.V. Jelley, Cherenkov radiation and its Applications, Pergamon Press, New York, 1958\n"
				<< "2. B.M. Bolotovskii \"Theory of Cherenkov radiation(III)\", Sov. Phys. Usp. 4(5), (1961) 781-811\n"
				<< "3. B.M. Bolotovskii \"Vavilov-Cherenkov radiation: its discovery and application\" Phys. Usp. 52(11), (2009) 1099-1110\n"
				<< "4. A.P. Kobzev \"On the radiation mechanism of a uniformly moving charge\", Phys. Part. Nucl. 45(3), (2014) 628-653\n\n"
				<< "End of CherenkovProcess<T>::ProcessDescription()\n";
	outStream << std::setw(116) << '\n';
	outStream << std::endl;
}

template<typename T>
void CherenkovProcess<T>::BuildWorkerPhysicsTable(const G4ParticleDefinition& aParticle) {
	for (auto& [key, model] : m_registeredModels)
		model->BuildWorkerModelPhysicsTable(aParticle);
}

template <typename T>
void CherenkovProcess<T>::PrepareWorkerPhysicsTable(const G4ParticleDefinition& aParticle) {
	for (auto& [key, model] : m_registeredModels)
		model->PrepareWorkerModelPhysicsTable(aParticle);
}

//=========protected ChR::CherenkovProcess<T>:: methods=========
template<typename T>
double CherenkovProcess<T>::GetMeanFreePath(const G4Track&, double, G4ForceCondition*) { //for now never used
	return -1.;
}

//=========private ChR::CherenkovProcess<T>:: methods=========


//=========full explicit specializations for ChR::CherenkovProcess<ChRModelIndex>:: methods (only needed ones)=========
template<> CherenkovProcess<ChRModelIndex>::CherenkovProcess(std::string);
template<> double CherenkovProcess<ChRModelIndex>::PostStepGetPhysicalInteractionLength(const G4Track&, double, G4ForceCondition*);
template<> G4VParticleChange* CherenkovProcess<ChRModelIndex>::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep);
template<> void CherenkovProcess<ChRModelIndex>::StartTracking(G4Track* aTrack);

endChR

#endif // !CherenkovProcess_hpp