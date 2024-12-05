//##########################################
//#######         VERSION 0.6        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

#pragma once
#ifndef ProcessCsvData_hpp
#define ProcessCsvData_hpp

/*
ABOUT THE HEADER FILE:
In this header I provide a variadic template class ProcessCsvData<Args...> and some helper functions
that are used by the class methods. For now, I keep the goal of this class very simple - do counting
and prepare output files (.csv as well) for obtaining graphs in N dimensions. Thus, for now, the count
value is used as a dependent, while N dimensions are independent. Also, according to G4AnalysisManager
only four data types can be accepted - int, double, float, and std::string. Note, even though it's just
a counter class, in a few lines of code that can change (see the NOTE "countVector").
One should run this class after acquiring all data and closing G4AnalysisManager NTuple files. Thus,
as only the Master thread can run this class, the ideal place to run this class would be EndOfRunAction,
but for the master thread:

void RunAction::EndOfRunAction(const G4Run* aRun) {
	G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
	analysisManager->Write();
	analysisManager->CloseFile();
	//Some work
	if (isMaster) {
		if (fileExtension == "csv") {
			ProcessCsvData<*tuple ID types*> processCsv{fileName, fileExtension, ntupleName, *verboseLevel*, *outputFolderName*};
			processCsv.MoveAFileToFinalDestination("*Some additional file that was generated*");
			processCsv.Process_N_D_Data<*order of unique ntuple column IDs*>(std::vector<double>{*bin Values*}, "*output file name*");
			processCsv.Process_N_D_Data...
			...
		}
	}
}

Also, by specializing ReadMePrintAboutCurrentProjectData one can print all about
used parameters in simulations. Note that private members "p_thrPool" and "m_taskGroup"
can be used to extract information from objects that are only in worker_thread scope.
For instance,
   "p_thrPool->execute_on_all_threads( *lambda function* )"
will execute a lambda function by all threads, while
   "m_taskGroup.exec( *lambda function* });
	m_taskGroup.wait();"
will add a task (lambda function) which will be execute by a single thread of the registered threads in PTL::ThreadPool.
*/

//User built headers
#include "DefsNConsts.hpp"
#include "DefsNConsts.hpp"
//G4 headers
#include "G4Threading.hh"
#include "G4Filesystem.hh"
#include "PTL/TaskRunManager.hh"
#include "PTL/ThreadPool.hh"
#include "G4RunManager.hh"
#include "G4ThreadPool.hh"
#include "G4TaskGroup.hh"
#include "SomeGlobalNamespace.hh"
//std:: headers
#include <tuple>
#include <type_traits>
#include <vector>
#include <string>
#include <string_view>
#include <chrono>
#include <fstream>
#include <string>
#include <mutex>
#include <algorithm>
#include <vector>
#include <set>

beginChR

//not a very RAM-friendly class (I still might improve it in the future)...
//preferably to be used in the user's master EndOfRunAction method
//Also, make sure to keep this class in a scope duration
#if _HAS_CXX20
template <typename... Args>
concept G4AnslysisTypes =
((std::is_same_v<Args, std::string> || std::is_same_v<Args, double> || std::is_same_v<Args, float> || std::is_same_v<Args, int>) && ...) &&
sizeof...(Args) > 0;

template<typename... Args>
	requires G4AnslysisTypes<Args...>
class ProcessCsvData {
#else // C++17 - I'm going with static_assert for shorter code, might change to enable_if (too verbose I think)
template <typename... Args>
class ProcessCsvData {
	static_assert(
		(((std::is_same_v<Args, std::string> || std::is_same_v<Args, double> || std::is_same_v<Args, float> || std::is_same_v<Args, int>) && ...) &&
			sizeof...(Args) > 0),
		"G4AnalysisManager allows to write only string, double, float or int nTuples. Therefore, you must use those types in this class. Also, No. of Args must be greater than 0"
		);
#endif // _HAS_CXX20

private:
	/*
	Pay attention here!! I wrote this function here so one would notice it! The user should specialize this function for his/her
	own project. Otherwise ReadMe file won't contain much information. Basically, some general information are written by default,
	and then std::ofstream object is passed to this function by reference, i.e., the user can write his/her own data.
	*/
	void ReadMePrintAboutCurrentProjectData(std::ofstream&);

public:
	using tuple_t = std::tuple<Args...>;
	using iterator_t = typename std::vector<tuple_t>::iterator;

	//The constructor prepares an output folder moves G4AnalysisManager .csv files, loads data from them and writes ReadMe file
	ProcessCsvData(std::string_view fileName, std::string_view ntupleName, unsigned char verbose = 0, const char* aDIRName = "");

	virtual ~ProcessCsvData() = default;
	ProcessCsvData() = delete;

	//move/copy of objects would be very unwise due as they could be very memory heavy
	ProcessCsvData(const ProcessCsvData& other) = delete;
	ProcessCsvData& operator=(ProcessCsvData& other) = delete;
	ProcessCsvData(ProcessCsvData&& other) = delete;
	ProcessCsvData& operator=(ProcessCsvData&& other) = delete;

	/*
	Used if one would like to move some specific files from working .exe folder to the newly created folder.
	*/
	void MoveAFileToFinalDestination(const char*);
	/*
	Load new data (this function is used in constructor, but one may reuse it, e.g., in a case of more than 1 tuple ID)
	*/
	inline void LoadNewData(std::string_view fileName, std::string_view ntupleName);

	/*
	The following function is used to produce output files with the processed data. Of course, this class performs just initial
	processing, while the obtained .csv files can be further used by some other programs. In the function, template parameters
	are used as ID columns of NTuples obtained by G4AnalysisManager. Template parameters must be unique, and their order is important.
	For instance, that means writing <0, 5, 3, 2> will produce 4D data (4 dimensions and count) while ordering will be 0 -> 5 -> 3 -> 2
	Next, vectorOfBinValues is used to pass required bin values of output plots. The result of the function will be vectorOfBinValues.size()
	files holding processed data for various bin values. CARE, CURRENTLY I DID NOT PROVIDE ANY SAFETIES, SO IF THERE'S A FILE WITH SPECIFIC
	NAME ALREADY, IT WILL BE REMOVED!!!
	*/
	template <size_t... tupleSortingOrder_N_Dimensions>
	void Process_N_D_Data(const std::vector<double>& vectorOfBinValues, const char* outFileName, const char* efficiencyFile = "");

protected:
	//Initial check of used template parameters in Process_N_D_Data
	template <size_t... RestDim>
	constexpr void DoCheckUpOfInput() const;

	//This is the method that call for ReadMePrintAboutCurrentProjectData
	virtual void WriteReadMeFile();

	//Member variables
	std::vector<tuple_t> m_dataVec{};
	std::vector<std::pair<double, double>> m_efficiencyValues;
	std::string m_previousEfficiencyFile;
	G4fs::path m_currentPath;
	G4fs::path m_newDIRPath;
	std::mutex m_workerThreadMutex;
	unsigned int m_verboseLevel;
	bool m_efficiencyFlag;

	/*
	Haven't decided yet if this class should be inherited, but, for now, keeping the following protected just in case...
	The following methods basically do all the serious part of the class and are used by "Process_N_D_Data"
	*/
	template <bool alreadyFoundMax, size_t NextDim, size_t... RestDim>
	void PrepareBinVectors(std::vector<std::vector<double>>&, const double binWidth, size_t& countNo, double maxValue);
	template <size_t NextDim, size_t... RestDim>
	void SortDataVecForCounting(std::vector<std::vector<double>>&, const iterator_t begin, const iterator_t end);
	template <size_t NextDim, size_t... RestDim>
	void CountFinalData(const std::vector<std::vector<double>>&, std::vector<size_t>&, iterator_t, iterator_t, const size_t);
	template <size_t NextDim, size_t... RestDim>
	void PrintOutputData(const std::vector<std::vector<double>>&, std::vector<size_t>&, std::ofstream&, const size_t multiplier, std::string&, std::ostringstream&);
private:
	void WorkersToLoadCsvFiles(std::string_view fileName, std::string_view ntupleName);
	template <typename T, typename... SubArgs>
	void PushLineToTuple(std::istringstream&, tuple_t&, const std::string&);
	//for fast access of threads registered in the ThreadPool
	PTL::ThreadPool* p_thrPool = nullptr;
	PTL::TaskGroup<void> m_taskGroup;
};

//
//=========additional namespace functions needed in ChR::ProcessCsvData<Args...>:: methods=========
//

//specializing AssignValue because the compiler complains about passing string...
//like this, the string actually doesn't exists till it's needed
template <typename T>
inline void AssignValue(T& assign, const std::string& aValue) {}
template<>
inline void AssignValue<double>(double& assign, const std::string& aValue) {
	assign = std::stod(aValue);
}
template<>
inline void AssignValue<float>(float& assign, const std::string& aValue) {
	assign = std::stof(aValue);
}
template<>
inline void AssignValue<int>(int& assign, const std::string& aValue) {
	assign = std::stoi(aValue);
}
template<>
inline void AssignValue<std::string>(std::string& assign, const std::string& aValue) {
	assign = aValue;
}



template <size_t LeadElement, size_t CompareElement, size_t... RestArgs>
[[nodiscard]] constexpr bool CheckPackUniqueness() {
	if constexpr (LeadElement != CompareElement) {
		if constexpr (sizeof...(RestArgs) != 0)
			if constexpr (CheckUniqueness<LeadElement, RestArgs...>())
				return CheckUniqueness<CompareElement, RestArgs...>();
			else
				return false;
		else
			return true;
	}
	else
		return false;
}



//passing pointers to the two following functions for various algorithm comparisons
template <size_t I, typename... Args>
[[nodiscard]] inline bool LessCompareSpecificTupleValue(const std::tuple<Args...>& first, const std::tuple<Args...>& second) {
	return std::get<I>(first) < std::get<I>(second);
}
template <size_t I, typename... Args>
[[nodiscard]] inline bool GreaterCompareSpecificTupleValue(const std::tuple<Args...>& first, const std::tuple<Args...>& second) {
	return LessCompareSpecificTupleValue(second, first);
}
template <size_t I, typename... Args>
[[nodiscard]] inline bool LessCompareSpecificTupleWithADoubleValue(const std::tuple<Args...>& first, const double second) {
	return std::get<I>(first) < second;
}
template <size_t I, typename... Args>
[[nodiscard]] inline bool GreaterCompareSpecificTupleWithADoubleValue(const std::tuple<Args...>& first, const double second) {
	return std::get<I>(first) > second;;
}



//
//=========private ChR::ProcessCsvData<Args...>::ReadMePrintAboutCurrentProjectData method=========
//

#if _HAS_CXX20
template<typename... Args>
	requires G4AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
void ProcessCsvData<Args...>::ReadMePrintAboutCurrentProjectData(std::ofstream& outS) {
	outS << "You need to specialize the \"ProcessCsvData<Args...>::ReadMePrintAboutCurrentProjectData\" method to print ReadMe information about your project!\n";
}

//
//=========public ChR::ProcessCsvData<Args...>:: methods=========
//

#if _HAS_CXX20
template<typename... Args>
	requires G4AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
ProcessCsvData<Args...>::ProcessCsvData(std::string_view fileName, std::string_view ntupleName, unsigned char verbose, const char* aDIRName)
: m_verboseLevel(verbose), m_efficiencyFlag(false),
p_thrPool(PTL::TaskRunManager::GetInstance()->GetThreadPool()), m_taskGroup() {
	std::string outDIR{ aDIRName };
	m_dataVec.reserve(1000000);
	if (outDIR.empty()) {
		const long long currTime = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()).time_since_epoch().count();
		outDIR = std::to_string(currTime);
	}
	if (!G4Threading::IsMasterThread())
		G4Exception("ProcessCsvData<Args...>::ProcessCsvData", "FE_ProcCsvData01", FatalException, "You MUST use this class with the master thread!");
	m_currentPath = G4fs::current_path();
	m_newDIRPath = m_currentPath;
	m_newDIRPath /= outDIR;
	G4fs::create_directory(m_newDIRPath);
	LoadNewData(fileName, ntupleName);
	WriteReadMeFile();
}



#if _HAS_CXX20
template<typename... Args>
	requires G4AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
void ProcessCsvData<Args...>::MoveAFileToFinalDestination(const char* aFile) {
	m_currentPath /= aFile;
	if (!G4fs::exists(m_currentPath)) {
		if (m_verboseLevel > 0) {
			std::ostringstream err;
			err << "File " << std::quoted(aFile) << " was not found on the path:\n" << m_currentPath << "\nand was not moved!\n";
			G4Exception("ProcessCsvData<Args...>::MoveAFileToFinalDestination", "WE_ProcCsvData01", JustWarning, err);
		}
		m_currentPath.remove_filename();
		return;
	}
	m_newDIRPath /= aFile;
	try {
		G4fs::rename(m_currentPath, m_newDIRPath);
	}
	catch (...) {
		std::ostringstream err;
		err << "File could not be moved to the proper folder for some reason, e.g., file in use... The file path is:\n"
			<< m_currentPath << '\n';
		G4Exception("ProcessCsvData<Args...>::MoveAFileToFinalDestination", "WE_ProcCsvData02", JustWarning, err);
	}
	m_newDIRPath.remove_filename();
	m_currentPath.remove_filename();
}



#if _HAS_CXX20
template<typename... Args>
	requires G4AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
void ProcessCsvData<Args...>::LoadNewData(std::string_view fileName, std::string_view ntupleName) {
	if (!m_dataVec.empty()) {
		m_dataVec.clear();
		if (m_verboseLevel >= 1)
			std::cout << "All data loaded in the ProcessCsvData<Args...> object have been removed!\n";
	}
	if (m_verboseLevel >= 1)
		std::cout << "Preparing to load new data in the ProcessCsvData<Args...> object!\n";
	p_thrPool->execute_on_all_threads([&] { WorkersToLoadCsvFiles(fileName, ntupleName); });
}



#if _HAS_CXX20
template<typename... Args>
	requires G4AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
template <size_t... Dimensions>
void ProcessCsvData<Args...>::Process_N_D_Data(const std::vector<double>& arg_binVector, const char* aFileName, const char* efficiencyFile) {
	//first check if everything is ok for processing
	double maxValue = DBL_MAX;
	DoCheckUpOfInput<Dimensions...>();
	//Now if efficiency should be used
	if constexpr (sizeof...(Dimensions) != 1) {
		m_efficiencyFlag = false;
		if (efficiencyFile[0] == '\0') {
			if (m_verboseLevel >= 2) {
				const char* err = "Efficiency file not specified - not using efficiency\n";
				G4Exception("ProcessCsvData<Args...>::Process_N_D_Data", "WE_ProcCsvData04", JustWarning, err);
			}
		}
		else {
			if (m_verboseLevel >= 1) {
				const char* err = "Currently, efficiency can be used only for a 1D case!\nThe data are not being processed!\n";
				G4Exception("ProcessCsvData<Args...>::Process_N_D_Data", "WE_ProcCsvData05", JustWarning, err);
				return;
			}
		}
	}
	else {
		//Always looking for a new maxValue - the dimension might have changed
		maxValue = std::get<Dimensions...>(*std::max_element(m_dataVec.begin(), m_dataVec.end(), &LessCompareSpecificTupleValue<Dimensions..., Args...>));
		if (efficiencyFile[0] == '\0') {
			m_efficiencyFlag = false;
			if (m_verboseLevel >= 2) {
				const char* err = "Efficiency file not specified - not using efficiency\n";
				G4Exception("ProcessCsvData<Args...>::Process_N_D_Data", "WE_ProcCsvData06", JustWarning, err);
			}
		}
		else
			m_efficiencyFlag = true;
	}
	if (m_efficiencyFlag) {
		if (!m_efficiencyValues.empty() && m_previousEfficiencyFile == efficiencyFile)
			goto AlreadyLoadedEfficiency;
		m_efficiencyValues.clear();
		m_previousEfficiencyFile = efficiencyFile;
		m_efficiencyValues.reserve(150); //I think 150 should be enough for most cases
		m_currentPath /= efficiencyFile;
		if (m_currentPath.extension() != ".csv") {
			std::ostringstream err;
			err << "The provided file " << std::quoted(efficiencyFile) << " that contains efficiency"
				<< "\ndoes not have .csv extension! The data are not processed!\nThe path of the erroneous file is:\n"
				<< m_currentPath << '\n';
			G4Exception("ProcessCsvData<Args...>::Process_N_D_Data", "WE_ProcCsvData07", JustWarning, err);
			m_currentPath.remove_filename();
			return;
		}
		else {
			std::ifstream iFS{ m_currentPath, std::ios::in };
			if (!iFS.is_open()) {
				std::ostringstream err;
				err << "The provided file " << std::quoted(efficiencyFile) << " that contains efficiency"
					<< "\ncould not be opened! The data are not processed!\nThe path of the erroneous file is:\n"
					<< m_currentPath << '\n';
				G4Exception("ProcessCsvData<Args...>::Process_N_D_Data", "WE_ProcCsvData08", JustWarning, err);
				m_currentPath.remove_filename();
				return;
			}
			//for now loading data minimal safeties (using get lines allows some safeties while converting)!!
			//Might change it in the future... now it's user's responsibility
			std::string line, value;
			std::istringstream helperSStream;
			while (true) {
				std::getline(iFS, line, '\n');
				if (iFS.eof() || line.empty())
					break;
				helperSStream.str(line);
				helperSStream.clear();
				helperSStream.seekg(0);
				try {
					m_efficiencyValues.emplace_back();
					std::getline(helperSStream, value, ',');
					m_efficiencyValues.back().first = std::stod(value);
					std::getline(helperSStream, value, '\n');
					m_efficiencyValues.back().second = std::stod(value);
				}
				catch (...) {
					std::ostringstream err;
					err << "There's been an error while loading (converting) data from the efficiency provided file... The erroneous file path is:\n"
						<< m_currentPath << "\nThe requested data are not processed!\n";
					G4Exception("ProcessCsvData<Args...>::Process_N_D_Data", "WE_ProcCsvData09", JustWarning, err);
					m_efficiencyValues.clear();
					m_efficiencyFlag = false;
					m_currentPath.remove_filename();
					return;
				}
			}
			iFS.close();
		}
		if (m_efficiencyValues.size() < 2) {
			std::ostringstream err;
			err << "Too few efficiency data loaded! The requested data are not processed! The used file path is:\n"
				<< m_currentPath << '\n';
			G4Exception("ProcessCsvData<Args...>::Process_N_D_Data", "WE_ProcCsvData10", JustWarning, err);
			m_efficiencyValues.clear();
			m_efficiencyFlag = false;
			m_currentPath.remove_filename();
			return;
		}
		if (!std::is_sorted(m_efficiencyValues.begin(), m_efficiencyValues.end(),
			[](std::pair<double, double> firstPair, std::pair<double, double> secondPair) {return firstPair.first < secondPair.first; })) {
			std::ostringstream err;
			err << "The data loaded from the path file:\n"
				<< m_currentPath << "\nare not sorted in an ascending order! The requested data are not processed!\n";
			G4Exception("ProcessCsvData<Args...>::Process_N_D_Data", "WE_ProcCsvData11", JustWarning, err);
			m_efficiencyValues.clear();
			m_efficiencyFlag = false;
			m_currentPath.remove_filename();
			return;
		}
		m_currentPath.remove_filename();
	}
AlreadyLoadedEfficiency:
	std::ofstream outFileData;
	for (double binWidth : arg_binVector) {
		if (binWidth <= 0)
			continue;
		std::ostringstream strStreamHelper;
		std::string outFileName = aFileName;
		strStreamHelper << std::fixed << std::setprecision(2) << binWidth;
		outFileName += "_Bin_" + strStreamHelper.str() + ".csv";
		/*
		NOTE "countVector": for now I'm using std::vector<size_t> because I'm currently keep this class
		as a counter class. However, I might change the size_t type in the future and then this
		class can easily change the entire point... Thus, placing a struct 'DataStruct' instead
		of size_t gives whatever data are needed without any problems.
		*/
		std::vector<size_t> countVector;
		/*
		binVectors are helper vectors. Each dimension has two associated helperVectors.
		The former one holds the printing value, i.e., the middle of the bin, while
		the latter vector holds bin Nodes, i.e., it has an extra element compared to the former vec
		*/
		std::vector<std::vector<double>> binVectors;
		binVectors.reserve(2 * sizeof...(Dimensions));
		for (size_t i = 0; i < 2 * sizeof...(Dimensions); i++)
			binVectors.emplace_back();
		size_t countNo = 1;
		if constexpr(sizeof...(Dimensions) == 1)
			PrepareBinVectors<true, Dimensions...>(binVectors, binWidth, countNo, maxValue);
		else /*(sizeof...(Dimensions...) != 1) -> maxValue not found!*/
			PrepareBinVectors<false, Dimensions...>(binVectors, binWidth, countNo, maxValue);
		if (m_efficiencyFlag && m_efficiencyValues.front().first > binVectors[1].front() && m_efficiencyValues.back().first < maxValue) {
			std::ostringstream err;
			err << "The efficiency data you loaded do not cover the whole range of data you are trying to use them on!\n"
				<< "Make sure to cover the whole expected range before using efficiency!\nThe requested data are not processed!\n";
			G4Exception("ProcessCsvData<Args...>::Process_N_D_Data", "WE_ProcCsvData12", JustWarning, err);
			m_efficiencyValues.clear();
			m_efficiencyFlag = false;
			return;
		}
		countVector.reserve(countNo + 1);
		for (size_t i = 0; i < countNo; i++)
			countVector.emplace_back(0);
		SortDataVecForCounting<Dimensions...>(binVectors, m_dataVec.begin(), m_dataVec.end());
		CountFinalData<Dimensions...>(binVectors, countVector, m_dataVec.begin(), m_dataVec.end(), 0ll);
		m_newDIRPath /= outFileName;
		outFileData.open(m_newDIRPath, std::ios::out | std::ios::trunc);
		m_newDIRPath.remove_filename();
		if (!outFileData.is_open()) {
			G4Exception("ProcessCsvData<Args...>::Process_N_D_Data", "WE_ProcCsvData13", JustWarning, "Output file not opened!\n");
			return;
		}
		strStreamHelper.precision(4);
		outFileData.precision(4);
		outFileData << std::fixed;
		std::string helperString{};
		PrintOutputData<Dimensions...>(binVectors, countVector, outFileData, 0, helperString, strStreamHelper);
		outFileData.close();
	}
}



//
//=========protected ChR::ProcessCsvData<Args...>:: methods=========
//

#if _HAS_CXX20
template<typename... Args>
	requires G4AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
template <size_t... RestDim>
constexpr void ProcessCsvData<Args...>::DoCheckUpOfInput() const {
	static_assert((!std::is_same_v<std::tuple_element_t<RestDim, tuple_t>, std::string> && ...),
		"The current version of the ProcessCsvData class does not support std::string processing! "
		"However, this feature will be added in the future (at least for 1D case). If you have any "
		"idea why I should make it possible to use N_D strings, please let me know via the provided "
		"contact. Remove the string component to successfully build the code!\n");
	if constexpr (sizeof...(RestDim) > 1) {
		static_assert(CheckPackUniqueness<RestDim...>(),
			"Non-unique elements detected in the parameter pack of a ProcessCsvData object! "
			"While calling methods to process N_D data, all parameter-pack values must be "
			"unique! That means, e.g., you can use <0, 1, 2, 5, 3>, but you cannot "
			"<0, 1, 2, 5, 1> ('1' is not unique)!\n");
	}
}



#if _HAS_CXX20
template<typename... Args>
	requires G4AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
void ProcessCsvData<Args...>::WriteReadMeFile() {
	std::ofstream writeReadMe;
	m_newDIRPath /= "ReadMe_OfThisRun.txt";
	writeReadMe.open(m_newDIRPath, std::ios::out | std::ios::trunc);
	m_newDIRPath.remove_filename();
	if (!writeReadMe) {
		G4Exception("ProcessCsvData<Args...>::WriteReadMeFile()", "WE_ProcCsvData14", JustWarning, "ReadMe file could not be opened!\n");
		return;
	}
	writeReadMe << G4RunManagerKernel::GetRunManagerKernel()->GetVersionString() << '\n'
		<< "The total number of events in this run was:  " << G4RunManager::GetRunManager()->GetNumberOfEventsToBeProcessed()
		<< "\nThe total number of threadPool threads was:  " << G4RunManager::GetRunManager()->GetNumberOfThreads() << std::endl;
	ReadMePrintAboutCurrentProjectData(writeReadMe);
	writeReadMe.close();
}



#if _HAS_CXX20
template<typename... Args>
	requires G4AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
template <bool alreadyFoundMax, size_t NextDim, size_t... RestDim>
void ProcessCsvData<Args...>::PrepareBinVectors(std::vector<std::vector<double>>& binVectors, const double binWidth, size_t& countNo, double maxValue) {
	if constexpr(!alreadyFoundMax)
		maxValue = std::get<NextDim>(*std::max_element(m_dataVec.begin(), m_dataVec.end(), &LessCompareSpecificTupleValue<NextDim, Args...>));
	double minValue = std::get<NextDim>(*std::min_element(m_dataVec.begin(), m_dataVec.end(), &LessCompareSpecificTupleValue<NextDim, Args...>));
	size_t noOfBin = (size_t)std::ceil((maxValue - minValue) / binWidth) + 1;
	//having one extra because comparisons are [range), i.e., in some cases the right value can be out of bounds and not in the bin range
	//that's the case if (maxValue - minValue) / binWidth returns an almost-integral number (it would be in '[range]', not '[range)')
	countNo *= noOfBin; //this one increases very quickly so one should be reasonable with binWidth for N_D analysis
	binVectors[binVectors.size() - 2 * (sizeof...(RestDim) + 1)].reserve(noOfBin);
	//not so friendly with memory, but going with +- 0.5binWidth might (I'm not sure) cause some super rare IEEE754 errors
	//I need precision here with no unnecessary + and - operations
	binVectors[binVectors.size() - 2 * sizeof...(RestDim) - 1].reserve(noOfBin + 1);
	for (size_t i = 0; i < noOfBin; i++) {
		binVectors[binVectors.size() - 2 * (sizeof...(RestDim) + 1)].emplace_back(minValue + ((double)i + 0.5) * binWidth);
		binVectors[binVectors.size() - 2 * sizeof...(RestDim) - 1].emplace_back(minValue + (double)i * binWidth);
	}
	binVectors[binVectors.size() - 2 * sizeof...(RestDim) - 1].emplace_back(minValue + (double)noOfBin * binWidth);
	if constexpr (sizeof...(RestDim) != 0) {
		PrepareBinVectors<false, RestDim...>(binVectors, binWidth, countNo, maxValue);
	}
}



#if _HAS_CXX20
template<typename... Args>
	requires G4AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
template <size_t NextDim, size_t... RestDim>
void ProcessCsvData<Args...>::SortDataVecForCounting(std::vector<std::vector<double>>& binVectors, const iterator_t begin, const iterator_t end) {
	std::sort(begin, end, &LessCompareSpecificTupleValue<NextDim, Args...>);
	if constexpr (sizeof...(RestDim) != 0) {
		iterator_t nextBegin = begin;
		for (size_t i = 1; i < binVectors[binVectors.size() - 2 * sizeof...(RestDim) - 1].size(); i++) {
			iterator_t nextEnd = std::lower_bound(nextBegin, end, binVectors[binVectors.size() - 2 * sizeof...(RestDim) - 1][i],
				&LessCompareSpecificTupleWithADoubleValue<NextDim, Args...>);
			SortDataVecForCounting<RestDim...>(binVectors, nextBegin, nextEnd);
			nextBegin = nextEnd;
		}
	}
}



#if _HAS_CXX20
template<typename... Args>
	requires G4AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
template <size_t NextDim, size_t... RestDim>
void ProcessCsvData<Args...>::CountFinalData(const std::vector<std::vector<double>>& binVectors,
	std::vector<size_t>& countVector, iterator_t begin, iterator_t end, const size_t multiplier) {
	/*
	only the last sorted dimension is used for counting here. On the other hand, multiplier passes the information
	from higher-order dimensions that helps to find the exact index where to change the counter.
	Besides that, iterators are used to divide the vector for the recursive-like thing of variadic templates
	*/
	if constexpr (sizeof...(RestDim) == 0) {
		size_t toAdd = 1;
		for (; begin != end; begin++) {
			while (true) {
				if (LessCompareSpecificTupleWithADoubleValue<NextDim, Args...>(*begin, binVectors.back()[toAdd])) {
					countVector[multiplier * binVectors[binVectors.size() - 2 * (sizeof...(RestDim) + 1)].size() + toAdd - 1]++;
					break;
				}
				else {
					toAdd++;
				}
				if (toAdd > binVectors.back().size()) {
					const char* err = "Fatal exception in the class algorithm logic... please report this problem via the provided contact\n";
					G4Exception("ProcessCsvData<Args...>::CountFinalData", "FE_ProcCsvData02", FatalException, err);
				}
			}
		}
	}
	else {
		iterator_t nextBegin = begin;
		for (size_t i = 1; i < binVectors[binVectors.size() - 2 * sizeof...(RestDim) - 1].size(); i++) {
			//the following should work even though vector is just partly sorted (it's sorted around significant values)
			iterator_t nextEnd = std::lower_bound(nextBegin, end, binVectors[binVectors.size() - 2 * sizeof...(RestDim) - 1][i],
				&LessCompareSpecificTupleWithADoubleValue<NextDim, Args...>);
			//finding the multiplier for the next dimension
			size_t toPassMultiplier = i + multiplier * binVectors[binVectors.size() - 2 * (sizeof...(RestDim) + 1)].size() - 1;
			CountFinalData<RestDim...>(binVectors, countVector, nextBegin, nextEnd, toPassMultiplier);
			nextBegin = nextEnd;
		}
	}
}



#if _HAS_CXX20
template<typename... Args>
	requires G4AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
template <size_t NextDim, size_t... RestDim>
void ProcessCsvData<Args...>::PrintOutputData(const std::vector<std::vector<double>>& binVectors, std::vector<size_t>& countVector, std::ofstream& outFile, const size_t multiplier, std::string& stringHelper, std::ostringstream& strStreamHelper) {
	/*
	The same logic as before, i.e., divide and use multiplier.
	The efficiency is included here. Still, might improve the efficiency system in the future
	*/
	for (size_t i = 0; i < binVectors[binVectors.size() - 2 * (sizeof...(RestDim) + 1)].size(); i++) {
		if constexpr (sizeof...(RestDim) == 0) {
			size_t coef = multiplier * (binVectors.end() - 2)->size() + i;
			if (!m_efficiencyFlag) {
				if (countVector[coef] != 0) {
					if (stringHelper.empty())
						outFile << (*(binVectors.end() - 2))[i] << ',' << countVector[coef] << '\n';
					else
						outFile << stringHelper << ',' << (*(binVectors.end() - 2))[i] << ',' << countVector[coef] << '\n';
				}
			}
			else {
				//stringHelper is always empty in this case because this works only for sizeof...(Dimensions) == 1
				if (countVector[coef] != 0) {
					try {
						//The following should work with some reasonable bin values and efficiency tables
						auto itr1 = std::lower_bound(m_efficiencyValues.begin(), m_efficiencyValues.end(), binVectors[0][i],
							[](std::pair<double, double> aPair, double value) {return aPair.first < value; });
						double efficiency = G4LinearInterpolate2D_GetY((itr1 - 1)->second, itr1->second, (itr1 - 1)->first, itr1->first, binVectors[0][i]);
						outFile << binVectors[0][i] << ',' << efficiency * (double)countVector[coef] << '\n';
					}
					catch (...) {
						G4Exception("ProcessCsvData<Args...>::PrintOutputData", "FE_ProcCsvData03", FatalException,
							"Error while printing data with efficiency! Please report the error!\n");
					}
				}
			}
		}
		else {
			//using helper strings to pass the information about the higher (non-last) dimensions
			strStreamHelper.str("");
			strStreamHelper << binVectors[binVectors.size() - 2 * (sizeof...(RestDim) + 1)][i]; //used to force precision
			if (stringHelper.empty())
				stringHelper += strStreamHelper.str();
			else
				stringHelper += ',' + strStreamHelper.str();
			size_t toPassMultiplier = i + multiplier * binVectors[binVectors.size() - 2 * (sizeof...(RestDim) + 1)].size();
			PrintOutputData<RestDim...>(binVectors, countVector, outFile, toPassMultiplier, stringHelper, strStreamHelper);
			auto sItr = stringHelper.rfind(',');
			if (sItr != std::string::npos)
				stringHelper.erase(sItr);
			else
				stringHelper.clear();
		}
	}
}



//
//=========private ChR::ProcessCsvData<Args...>::ReadMePrintAboutCurrentProjectData method=========
//

#if _HAS_CXX20
template<typename... Args>
	requires G4AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
void ProcessCsvData<Args...>::WorkersToLoadCsvFiles(std::string_view argumentFileName, std::string_view ntupleName) {
	std::string filename{};
	filename += std::string{ argumentFileName.data() } + "_nt_" + std::string{ ntupleName.data() } + "_t" + std::to_string(G4Threading::G4GetThreadId()) + ".csv";
	G4fs::path localStartPath = m_currentPath;
	localStartPath /= filename;
	if (!G4fs::exists(localStartPath)) {
		if (m_verboseLevel > 0) {
			std::ostringstream err;
			err << "File for processing not found... the file path is:\n" << localStartPath << '\n';
			G4Exception("ProcessCsvData<Args...>::WorkersToLoadCsvFiles()", "WE_ProcCsvData15", JustWarning, err);
			return;
		}
	}
	G4fs::path localEndPath = m_newDIRPath;
	localEndPath /= filename;
	try {
		G4fs::rename(localStartPath, localEndPath);
	}
	catch (...) {
		std::ostringstream err;
		err << "File could not be moved to the proper folder for some reason, e.g., file in use... most likely, the processed data won't be correct! The file path is:\n"
			<< localStartPath << '\n';
		G4Exception("ProcessCsvData<Args...>::WorkersToLoadCsvFiles()", "WE_ProcCsvData16", JustWarning, err);
		return;
	}
	std::ifstream ifS;
	ifS.open(localEndPath, std::ios::in);
	if (!ifS) { //shouldn't happen
		std::ostringstream err;
		err << "File could not be opened. The file path is:\n"
			<< localStartPath << '\n';
		G4Exception("ProcessCsvData<Args...>::WorkersToLoadCsvFiles()", "WE_ProcCsvData17", JustWarning, err);
		return;
	}
	std::istringstream iSS;
	std::string line;
	std::vector<tuple_t> localVector;
	localVector.reserve(500);
	while (true) {
		if (ifS.eof()) {
			ifS.close();
			break;
		}
		if (localVector.size() == localVector.capacity()) { //move data to the member vector
			std::lock_guard lck{ m_workerThreadMutex };
			m_dataVec.insert(m_dataVec.end(), localVector.begin(), localVector.end());
			localVector.clear();
		}
		std::getline(ifS, line, '\n');
		if (line.front() == '#' || line.empty())
			continue;
		localVector.emplace_back();
		iSS.str(line);
		iSS.clear();
		iSS.seekg(0);
		PushLineToTuple<Args...>(iSS, localVector.back(), filename);
	}
	if (!localVector.empty()) {
		std::lock_guard lck{ m_workerThreadMutex };
		m_dataVec.insert(m_dataVec.end(), localVector.begin(), localVector.end());
	}
}



#if _HAS_CXX20
template<typename... Args>
	requires G4AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
template <typename T, typename... SubArgs>
void ProcessCsvData<Args...>::PushLineToTuple(std::istringstream& iSS, tuple_t& aTuple, const std::string& fileName) {
	std::string value{};
	std::getline(iSS, value, ',');
	auto FigureData = [&] {
		try {
			if constexpr (std::is_same_v<std::tuple_element_t<sizeof...(Args) - sizeof...(SubArgs) - 1, tuple_t>, double>) {
				AssignValue(std::get<sizeof...(Args) - sizeof...(SubArgs) - 1>(aTuple), value);
			}
			else if constexpr (std::is_same_v<std::tuple_element_t<sizeof...(Args) - sizeof...(SubArgs) - 1, tuple_t>, float>) {
				AssignValue(std::get<sizeof...(Args) - sizeof...(SubArgs) - 1>(aTuple), value);
			}
			else if constexpr (std::is_same_v<std::tuple_element_t<sizeof...(Args) - sizeof...(SubArgs) - 1, tuple_t>, int>) {
				AssignValue(std::get<sizeof...(Args) - sizeof...(SubArgs) - 1>(aTuple), value);
			}
			else /*std::string*/ {
				AssignValue(std::get<sizeof...(Args) - sizeof...(SubArgs) - 1>(aTuple), value); //AssignValue is needed because of this one
			}
		}
		catch (...) { //for now going with "something"... in the future might improve
			std::ostringstream err;
			err << "File corrupted... something went wrong while reading data from your output file. The erroneous file name is:\n" << fileName << '\n';
			G4Exception("ProcessCsvData<Args...>::PushLineToTuple", "FE_ProcCsvData04", FatalException, err);
		}
		};
	if constexpr (sizeof...(SubArgs) == 0) {
		FigureData();
	}
	else {
		FigureData();
		PushLineToTuple<SubArgs...>(iSS, aTuple, fileName);
	}
}

endChR

#endif // !ProcessCsvData_hpp