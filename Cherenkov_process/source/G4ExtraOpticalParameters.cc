//##########################################
//#######         VERSION 0.5        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

//G4 headers
#include "G4ExtraOpticalParameters.hh"
#include "G4ExtraOpticalParameters_Messenger.hh"
#include "G4AccessPhysicsVectors.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4Material.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ProcessTable.hh"
#include "G4CherenkovProcess.hh"
#include "G4SystemOfUnits.hh"

//=========public G4ExtraOpticalParameters:: methods=========

std::shared_ptr<G4ExtraOpticalParameters> G4ExtraOpticalParameters::GetInstance() {
	static std::shared_ptr<G4ExtraOpticalParameters> instance{ new G4ExtraOpticalParameters{} };
	return instance;
}

G4ExtraOpticalParameters::~G4ExtraOpticalParameters() {
	delete p_extraOpticalParameters_Messenger;
}

void G4ExtraOpticalParameters::ScanAndAddUnregisteredLV() {
	G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
	for(G4LogicalVolume* logVolume : *lvStore) {
		G4CherenkovMatData& matData = m_ChRMatData[logVolume];
		if (!matData.m_exoticRIndex) {
			const G4MaterialPropertiesTable* matPropTab = logVolume->GetMaterial()->GetMaterialPropertiesTable();
			if (matPropTab) {
				G4PhysicsFreeVector* physVec = matPropTab->GetProperty(kRINDEX);
				if (!physVec)
					physVec = matPropTab->GetProperty(kREALRINDEX);
				if (physVec) {
					const auto& dataVec = reinterpret_cast<const G4AccessPhysicsVector*>(physVec)->GetDataVector();
					// now, keeping the condition simple... might change in the future
					if (!std::is_sorted(dataVec.begin(), dataVec.end())) { //normally it should be sorted in ascending order
						matData.m_exoticRIndex = true;
						matData.m_exoticFlagInital = true;
					}
				}
			}
		}
	}
}

void G4ExtraOpticalParameters::PrintChRMatData(const G4LogicalVolume* aLogicalVolume) const {
	auto PrintLVData =
	[](const G4LogicalVolume* key, const G4CherenkovMatData& aMatData) {
		const G4CherenkovProcess* chProc = dynamic_cast<const G4CherenkovProcess*>(G4ProcessTable::GetProcessTable()->FindProcess("Cherenkov", "e-"));
		std::cout.fill(' ');
		std::cout << "G4CherenkovMatData for logical volume " << std::quoted(key->GetName()) << ":\n"
			<< std::setw(39) << ' ' << std::setfill('^') << std::setw(key->GetName().length() + 1) << '\n' << std::setfill(' ')
			<< std::left << std::setw(31) << "Selected process: " << aMatData.m_executeModel
			<< " (" << chProc->GetChRModel(aMatData.m_executeModel)->GetChRModelName() << ")\n"
			<< std::setw(31) << "Thickness:" << aMatData.m_matThickness / um << " um\n"
			<< std::setw(31) << "Current exotic RIndex flag:";
		if (aMatData.m_exoticRIndex)
			std::cout << "true\n";
		else
			std::cout << "false\n";
		std::cout << std::setw(31) << "Initial exotic RIndex flag:";
		if (aMatData.m_exoticFlagInital)
			std::cout << "true\n";
		else
			std::cout << "false\n";
	};
	std::cout.fill('=');
	std::cout << std::defaultfloat << std::setw(51) << '\n'
		<< "Begin of G4ExtraOpticalParameters::PrintChRMatData\n\n";
	if (aLogicalVolume) {
		const G4CherenkovMatData* aMatData = FindChRMatData(aLogicalVolume);
		PrintLVData(aLogicalVolume, *aMatData);
		std::cout << "\nEnd of G4ExtraOpticalParameters::PrintChRMatData\n"
			<< std::right << std::setfill('=') << std::setw(51) << '\n';
		return;
	}
	std::cout << std::setfill('+') << std::setw(41) << '\n';
	for (auto& [key, value] : m_ChRMatData) {
		PrintLVData(key, value);
		std::cout << std::right << std::setfill('+') << std::setw(41) << '\n';
	}
	std::cout << "\nEnd of G4ExtraOpticalParameters::PrintChRMatData\n"
		<< std::right << std::setfill('=') << std::setw(51) << '\n';
}

//=========private G4ExtraOpticalParameters:: methods=========

G4ExtraOpticalParameters::G4ExtraOpticalParameters() {
	p_extraOpticalParameters_Messenger = new G4ExtraOpticalParameters_Messenger{ this };
	G4OpticalParameters::Instance(); //basically, it does nothing (the object is just instantiated sooner)
}