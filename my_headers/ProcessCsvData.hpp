#pragma once
#ifndef ProcessCsvData_hpp
#define ProcessCsvData_hpp

//##########################################
//#######         VERSION 0.2        #######
//#######    Used: Geant4 v11.1 MT   #######
//#######   Tested on MSVC compiler  #######
//#######    Author: Djurnic Blazo   #######
//####### Contact: zobla96@gmail.com #######
//##########################################

/*
ABOUT THE HEADER FILE:
In this header I provide a variadic template class ProcessCsvData<Args...> and some helper functions
that are used by the class methods. For now, I keep the goal of this class very simple - do counting
and prepare output files (.csv as well) for obtaining graphs in N dimensions. Thus, for now, the count
value is used as a dependant, while N dimensions are independants. Also, according to G4AnalysisManager
only four data types can be accepted - int, double, float, and std::string. Note, even though it's just
a counter class, in a few lines of code that can change (see NOTE in line 493).
One should run this class after accuiring all data and closing G4AnalysisManager NTuple files. Thus,
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
			processCsv.Proces_N_D_Data<*order of unique ntuple column IDs*>(std::vector<double>{*bin Values*}, "*output file name*");
			processCsv.Proces_N_D_Data...
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
//std:: headers
#include <tuple>
#include <type_traits>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <string>
#include <mutex>
#include <algorithm>
#include <vector>
#include <set>

beginChR

//not a very RAM-friendly class... preferably to be used in the user's master EndOfRunAction method
//Also, make sure to keep this class in a scope duration
//It's very possible this is not the final version of this class and might be improved in the future
#if _HAS_CXX20
template <typename... Args>
concept G4_AnslysisTypes = 
	((std::is_same_v<Args, std::string> || std::is_same_v<Args, double> || std::is_same_v<Args, float> || std::is_same_v<Args, int>) && ...) &&
	sizeof...(Args) > 0;

template<typename... Args>
requires G4_AnslysisTypes<Args...>
class ProcessCsvData {
#else // C++17 - I'm going with static_assert for shorter code, might change to enable_if (too verbose I think)
template <typename... Args>
class ProcessCsvData {
	static_assert(
		(((std::is_same_v<Args, std::string> || std::is_same_v<Args, double> || std::is_same_v<Args, float> || std::is_same_v<Args, int>) && ...) &&
		sizeof...(Args) > 0),
		"G4AnlysisManager allows to write only string, double, float or int nTuples. Therefore, you must use those types in this class. Also, No. of Args must be greater than 0"
		);
#endif // _HAS_CXX20
public:
	//two aliases to reduce verbosity
	using tuple_t = std::tuple<Args...>;
	using iterator_t = typename std::vector<tuple_t>::iterator;

	//The constructor prepares an output folder moves G4AnalysisManager .csv files, loads data from them and writes ReadMe file
	ProcessCsvData(const std::string& fileName, const std::string& fileExtension, const std::string& ntupleName, unsigned char verbose = 1, const char* aDIRName = "");
	
	virtual ~ProcessCsvData() = default;
	ProcessCsvData() = delete;
	ProcessCsvData(const ProcessCsvData& other) = delete; //copy constructor would be wrong here
	ProcessCsvData& operator=(ProcessCsvData& other) = delete; //the same goes about copy assignment
	ProcessCsvData(ProcessCsvData&& other) = delete; //move constructor might be usable, but I prefer not to use it
	ProcessCsvData& operator=(ProcessCsvData&& other) = delete; //move assignment shouldn't be used due to const std::string& members... but again, I think one should not make more than one instance of this class.

	/*
	Pay attention here!! I wrote this function here so one would notice it! The user should specialize this function for his/her
	own project. Otherwise ReadMe file won't contain much infmation. Baseically, some general information are written by defauly,
	and then std::ofstream object is passed to this function by reference, i.e., the user can write his/her own data.
	*/
	void ReadMePrintAboutCurrentProjectData(std::ofstream&);

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
	void Proces_N_D_Data(const std::vector<double>& vectorOfBinValues, const char* outFileName, const char* efficiencyFile = "");

	/*
	Used if one would like to move some specific files from working .exe folder to the newly created folder.
	*/
	void MoveAFileToFinalDestination(const char*);
protected:
	//Initial check of used template parameters in Proces_N_D_Data
	template <size_t NextDim, size_t... RestDim>
	void DoCheckUpOfInput(std::vector<size_t>&) const;

	//This is the method that call for ReadMePrintAboutCurrentProjectData
	virtual void WriteReadMeFile();

	//Member variables
	std::vector<tuple_t> m_dataVec{};
	std::vector<std::pair<double, double>> m_efficiencyValues;
	//not sure if there's a safe way to get the data from G4AnalysisManager... the class semms quite locked
	//and only from the memory level I could access all the data. Thus, making it simple with names...
	G4fs::path m_currentPath;
	G4fs::path m_newDIRPath;
	const std::string& r_fileName;
	const std::string& r_fileExtension;
	const std::string& r_ntName;
	unsigned char m_verboseLevel;
	bool m_efficiencyFlag;
	std::mutex m_workerThreadMutex;

	/*
	Haven't decided yet if this class should be inherited, but, for now, keeeping the following protected just in case...
	The following methods basically do all the serious part of the class and are used by "Proces_N_D_Data"
	*/
	void WorkersToLoadCsvFiles();
	template <typename T, typename... SubArgs>
	void PushLineToTuple(std::istringstream&, tuple_t&, std::string&);
	template <size_t NextDim, size_t... RestDim>
	void PrepareBinVectors(std::vector<std::vector<double>>&, const double binWidth, size_t& countNo);
	template <size_t NextDim, size_t... RestDim>
	void SortDataVecForCounting(std::vector<std::vector<double>>&, const iterator_t begin, const iterator_t end);
	template <size_t NextDim, size_t... RestDim>
	void CountFinalData(const std::vector<std::vector<double>>&, std::vector<size_t>&, iterator_t, iterator_t, const size_t);
	template <size_t NextDim, size_t... RestDim>
	void PrintOutputData(const std::vector<std::vector<double>>&, std::vector<size_t>&, std::ofstream&, const size_t multiplier, std::string&, std::ostringstream&);
private:
	//for fast access of threads registered in the ThreadPool
	PTL::ThreadPool* p_thrPool = nullptr;
	PTL::TaskGroup<void> m_taskGroup;
};

#if _HAS_CXX20
template<typename... Args>
requires G4_AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
ProcessCsvData<Args...>::ProcessCsvData(const std::string& fName, const std::string& fExtension, const std::string& ntName, unsigned char verbose, const char* aDIR)
: r_fileName(fName), r_fileExtension(fExtension), r_ntName(ntName), m_verboseLevel(verbose), m_efficiencyFlag(false),
p_thrPool(PTL::TaskRunManager::GetInstance()->GetThreadPool()), m_taskGroup(p_thrPool) {
	std::string outDIR {aDIR};
	m_dataVec.reserve(1000000);
	if (outDIR.empty()) {
		//in the following line I use a MSVC macro, hope there's such in other compilers
		const size_t currTime = _CHRONO time_point_cast<_CHRONO seconds>(_CHRONO system_clock::now()).time_since_epoch().count();
		outDIR = std::to_string(currTime);
	}
	if (!G4Threading::IsMasterThread())
		G4Exception("ProcessCsvData<Args...>::ProcessCsvData()", "FE1023", FatalException, "You MUST use this class with the master thread!");
	if (fExtension != "csv")
		G4Exception("ProcessCsvData<Args...>::ProcessCsvData()", "FE1024", FatalException, "This class supports ONLY .csv files!");
	m_currentPath = G4fs::current_path();
	m_newDIRPath = m_currentPath;
	m_newDIRPath /= outDIR;
	G4fs::create_directory(m_newDIRPath);
	p_thrPool->execute_on_all_threads([&] {return WorkersToLoadCsvFiles(); });
	WriteReadMeFile();
}

#if _HAS_CXX20
template<typename... Args>
requires G4_AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
void ProcessCsvData<Args...>::ReadMePrintAboutCurrentProjectData(std::ofstream& outS) {
	outS << "You need to specialize the \"ProcessCsvData<Args...>::ReadMePrintAboutCurrentProjectData\" method to print ReadMe information about your project!\n";
}

#if _HAS_CXX20
template<typename... Args>
requires G4_AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
void ProcessCsvData<Args...>::WriteReadMeFile() {
	std::ofstream writeReadMe;
	m_newDIRPath /= "ReadMe_OfThisRun.txt";
	writeReadMe.open(m_newDIRPath, std::ios::out | std::ios::trunc);
	m_newDIRPath.remove_filename();
	if (!writeReadMe) {
		G4Exception("ProcessCsvData<Args...>::WriteReadMeFile()", "WE1024", JustWarning, "ReadMe file could not be opened!\n");
		return;
	}
	writeReadMe << G4RunManagerKernel::GetRunManagerKernel()->GetVersionString() << '\n'
		<< "The total number of events in this run was:  " << G4RunManager::GetRunManager()->GetNumberOfEventsToBeProcessed()
		<< "\nThe total number of threadPool threads was:  " << G4RunManager::GetRunManager()->GetNumberOfThreads()
		<< "\nThe total number of positive detections was: " << m_dataVec.size() << "\n\n";
	ReadMePrintAboutCurrentProjectData(writeReadMe);
	writeReadMe.close();
}

#if _HAS_CXX20
template<typename... Args>
requires G4_AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
void ProcessCsvData<Args...>::WorkersToLoadCsvFiles() {
	std::string filename{};
	filename += r_fileName + "_nt_" + r_ntName + "_t" + std::to_string(G4Threading::G4GetThreadId()) + '.' + r_fileExtension;
	G4fs::path localStartPath = m_currentPath;
	localStartPath /= filename;
	if (!G4fs::exists(localStartPath)) {
		std::ostringstream err;
		err << "File for processing not found... the file path is:\n" << localStartPath << '\n';
		G4Exception("ProcessCsvData<Args...>::WorkersToLoadCsvFiles()", "WE1021", JustWarning, err);
		return;
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
		G4Exception("ProcessCsvData<Args...>::WorkersToLoadCsvFiles()", "WE1022", JustWarning, err);
		return;
	}
	std::ifstream ifS;
	ifS.open(localEndPath, std::ios::in);
	if (!ifS) {
		std::ostringstream err;
		err << "File could not be opened. The file path is:\n"
			<< localStartPath << '\n';
		G4Exception("ProcessCsvData<Args...>::WorkersToLoadCsvFiles()", "WE1023", JustWarning, err);
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
			std::lock_guard lck{m_workerThreadMutex};
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
		std::lock_guard lck{m_workerThreadMutex};
		m_dataVec.insert(m_dataVec.end(), localVector.begin(), localVector.end());
	}
}

//specializing AssignValue because the compiler complains about passing string...
//like this the string actually doesn't exists till it's needed
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

#if _HAS_CXX20
template<typename... Args>
requires G4_AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
template <typename T, typename... SubArgs>
void ProcessCsvData<Args...>::PushLineToTuple(std::istringstream& iSS, tuple_t& aTuple, std::string& fileName) {
	std::string value{};
	std::getline(iSS, value, ',');
	auto FigureData = [&] {
		try {
			if constexpr (std::is_same_v<std::tuple_element_t<sizeof...(Args) - sizeof...(SubArgs) - 1, tuple_t>, double>) {
				AssignValue(std::get<sizeof...(Args) - sizeof...(SubArgs) - 1>(aTuple), value);
			}
			else if (std::is_same_v<std::tuple_element_t<sizeof...(Args) - sizeof...(SubArgs) - 1, tuple_t>, float>) {
				AssignValue(std::get<sizeof...(Args) - sizeof...(SubArgs) - 1>(aTuple), value);
			}
			else if (std::is_same_v<std::tuple_element_t<sizeof...(Args) - sizeof...(SubArgs) - 1, tuple_t>, int>) {
				AssignValue(std::get<sizeof...(Args) - sizeof...(SubArgs) - 1>(aTuple), value);
			}
			else /*std::string*/ {
				AssignValue(std::get<sizeof...(Args) - sizeof...(SubArgs) - 1>(aTuple), value); //AssignValue is needed because of this one
			}
		}
		catch (...) { //for now going with "something"... in the future might improve
			std::ostringstream err;
			err << "File corrupted... something went wrong while reading data from your output file. The erroneous file name is:\n" << fileName << '\n';
			G4Exception("ProcessCsvData<Args...>::WorkersToLoadCsvFiles()", "FE1025", FatalException, err);
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

//passing pointers to the two following functions for various algorithm comparisions
template <size_t I, typename... Args>
_NODISCARD inline bool LessCompareSpecificTupleValue(const std::tuple<Args...>& first, const std::tuple<Args...>& second) {
	return std::get<I>(first) < std::get<I>(second);
}
template <size_t I, typename... Args>
_NODISCARD inline bool GreaterCompareSpecificTupleValue(const std::tuple<Args...>& first, const std::tuple<Args...>& second) {
	return LessCompareSpecificTupleValue(second, first);
}
template <size_t I, typename... Args>
_NODISCARD inline bool LessCompareSpecificTupleWithADoubleValue(const std::tuple<Args...>& first, const double second) {
	return std::get<I>(first) < second;
}
template <size_t I, typename... Args>
_NODISCARD inline bool GreaterCompareSpecificTupleWithADoubleValue(const std::tuple<Args...>& first, const double second) {
	return std::get<I>(first) > second;;
}

#if _HAS_CXX20
template<typename... Args>
requires G4_AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
template <size_t... Dimensions>
void ProcessCsvData<Args...>::Proces_N_D_Data(const std::vector<double>& arg_binVector, const char* aFileName, const char* efficiencyFile) {
	//first check if everything is ok for processing
	try {
		std::vector<size_t> checkerVector{};
		DoCheckUpOfInput<Dimensions...>(checkerVector); //check for error input x_x
	}
	catch (std::ostringstream& err) {
		G4Exception("ProcessCsvData<Args...>::Proces_N_D_Data", "WE1035", JustWarning, err);
		return;
	}
	//Now if efficiency should be used
	if (sizeof...(Dimensions) != 1 || efficiencyFile[0] == '\0') {
		m_efficiencyFlag = false;
		//after the following one, I'm not considering verboseLevel but just printing warnings
		if (efficiencyFile[0] != '\0' && m_verboseLevel > 0) {
			const char* err = "Currently, efficiency can be use only for a single parameter till I figure out the point\nof using N_D efficiency. If you have some ideas, please let me know via the provided contact!\n";
			G4Exception("ProcessCsvData<Args...>::Proces_N_D_Data", "WE1028", JustWarning, err);
		}
	}
	else {
		m_efficiencyValues.reserve(150); //I think 150 should be enough for most cases
		m_efficiencyFlag = true;
	}
	if (m_efficiencyFlag) {
		m_currentPath /= efficiencyFile;
		if (m_currentPath.extension() != ".csv") {
			std::ostringstream err;
			err << "The provided file " << std::quoted(efficiencyFile)
				<< " does not have .csv extension! Proceeding without using efficiency!\nThe path of the erroneous file is:\n"
				<< m_currentPath << '\n';
			G4Exception("ProcessCsvData<Args...>::Proces_N_D_Data", "WE1029", JustWarning, err);
			m_efficiencyFlag = false;
		}
		else {
			std::ifstream iFS{m_currentPath};
			if (!iFS.is_open()) {
				std::ostringstream err;
				err << "The provided file " << std::quoted(efficiencyFile)
					<< " could not be opened! Proceeding without using efficiency!\nThe path of the erroneous file is:\n"
					<< m_currentPath << '\n';
				G4Exception("ProcessCsvData<Args...>::Proces_N_D_Data", "WE1030", JustWarning, err);
				m_efficiencyFlag = false;
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
				try	{
					m_efficiencyValues.emplace_back();
					std::getline(helperSStream, value, ',');
					m_efficiencyValues.back().first = std::stod(value);
					std::getline(helperSStream, value, '\n');
					m_efficiencyValues.back().second = std::stod(value);
				}
				catch (...) {
					std::ostringstream err;
					err << "There's been an error while loading (converting) data from the efficiency provided file... The erroneous file path is:\n"
						<< m_currentPath << "\nProceeding without using efficiency!\n";
					G4Exception("ProcessCsvData<Args...>::Proces_N_D_Data", "WE1031", JustWarning, err);
					m_efficiencyFlag = false;
					break;
				}
			}
			iFS.close();
		}
		if (m_efficiencyValues.size() < 2) {
			std::ostringstream err;
			err << "Too few efficiency data loaded! Proceeding wtihout using efficiency! The used file path is:\n"
				<< m_currentPath << '\n';
			G4Exception("ProcessCsvData<Args...>::Proces_N_D_Data", "WE1032", JustWarning, err);
			m_efficiencyFlag = false;
		}
		if (!std::is_sorted(m_efficiencyValues.begin(), m_efficiencyValues.end(),
			[](std::pair<double, double> firstPair, std::pair<double, double> secondPair) {return firstPair.first < secondPair.first; })) {
			std::ostringstream err;
			err << "The data loaded from the path file:\n"
				<< m_currentPath << "\nare not sorted in an ascending order! Proceeding wtihout using efficiency!\n";
			G4Exception("ProcessCsvData<Args...>::Proces_N_D_Data", "WE1033", JustWarning, err);
			m_efficiencyFlag = false;
		}
		m_currentPath.remove_filename();
	}
	std::ofstream outFileData;
	for (double binWidth : arg_binVector) {
		if (binWidth <= 0)
			continue;
		std::ostringstream strStreamHelper;
		std::string outFileName = aFileName;
		strStreamHelper << std::fixed << std::setprecision(2) << binWidth;
		outFileName += "_Bin_" + strStreamHelper.str() + '.' + r_fileExtension;
		std::vector<std::vector<double>> binVectors;
		/*
		NOTE HERE: for now I'm using std::vector<size_t> because I'm currently keep this class
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
		binVectors.reserve(2 * sizeof...(Dimensions));
		for (size_t i = 0; i < 2 * sizeof...(Dimensions); i++)
			binVectors.emplace_back();
		size_t countNo = 1.;
		PrepareBinVectors<Dimensions...>(binVectors, binWidth, countNo);
		if (m_efficiencyFlag && m_efficiencyValues.front().first < binVectors[1].front() && m_efficiencyValues.back().first > binVectors[1].back()) {
			std::ostringstream err;
			err << "The efficiency data you loaded do not cover the whole range of data you are trying to use them on!\n"
				<< "Make sure to cover the whole expected range before using efficiency!\nProceeding wtihout using efficiency!\n";
			G4Exception("ProcessCsvData<Args...>::Proces_N_D_Data", "WE1034", JustWarning, err);
			m_efficiencyFlag = false;
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
			G4Exception("ProcessCsvData<Args...>::Proces_N_D_Data", "WE1025", JustWarning, "Output file not opened!\n");
			return;
		}
		strStreamHelper << std::setprecision(4);
		outFileData.precision(4);
		outFileData << std::fixed;
		std::string helperString{};
		PrintOutputData<Dimensions...>(binVectors, countVector, outFileData, 0, helperString, strStreamHelper);
		outFileData.close();
	}
	m_efficiencyValues.clear();
}

#if _HAS_CXX20
template<typename... Args>
requires G4_AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
void ProcessCsvData<Args...>::MoveAFileToFinalDestination(const char* aFile) {
	m_currentPath /= aFile;
	if (!G4fs::exists(m_currentPath)) {
		if (m_verboseLevel > 0) {
			std::ostringstream err;
			err << "File " << std::quoted(aFile) << " was not found on the path:\n" << m_currentPath << "\nand was not moved!\n";
			G4Exception("ProcessCsvData<Args...>::MoveAFileToFinalDestination", "WE1026", JustWarning, err);
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
		G4Exception("ProcessCsvData<Args...>::MoveAFileToFinalDestination", "WE1027", JustWarning, err);
	}
	m_newDIRPath.remove_filename();
	m_currentPath.remove_filename();
}

#if _HAS_CXX20
template<typename... Args>
requires G4_AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
template <size_t NextDim, size_t... RestDim>
void ProcessCsvData<Args...>::DoCheckUpOfInput(std::vector<size_t>& checkerVector) const {
	checkerVector.emplace_back(NextDim);
	if (std::is_same_v<std::tuple_element_t<NextDim, tuple_t>, std::string>) {
		std::ostringstream err;
		err << "The current version of the class does not support std::string processing.\nHowever, this feautre will be added in the future, but only for 1D case.\nIf you have any why I should make it possible to use N_D strings, please let me know via the provided contact\nProceeding code without .csv data processing you requested!\n";
		throw err;
	}
	if constexpr (sizeof...(RestDim) == 0) {
		std::set<size_t> checkUniqueSet;
		for (size_t i : checkerVector)
			checkUniqueSet.insert(i);
		if (checkerVector.size() != checkUniqueSet.size()) {
			std::ostringstream err;
			err << "While calling methods to process data and obtain final N_D data, you must use unique non-type template parameter values!\n"
				<< "That means, e.g., you can use <0, 1, 2, 5, 3>, but you can't <0, 1, 2, 5, 1> ('1' is not unique)!\n"
				<< "However, your input was:\n<";
			for (auto i = checkerVector.begin(); i != checkerVector.end(); i++) {
				if (i == checkerVector.end() - 1)
					err << *i;
				else
					err << *i << ", ";
			}
			err << ">\nProceeding code without .csv data processing you requested!\n";
			throw err;
		}
	}
	else
		DoCheckUpOfInput<RestDim...>(checkerVector);
}

#if _HAS_CXX20
template<typename... Args>
requires G4_AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
template <size_t NextDim, size_t... RestDim>
void ProcessCsvData<Args...>::PrepareBinVectors(std::vector<std::vector<double>>& binVectors, const double binWidth, size_t& countNo) {
	double maxValue = std::get<NextDim>(*std::max_element(m_dataVec.begin(), m_dataVec.end(), &LessCompareSpecificTupleValue<NextDim, Args...>));
	double minValue = std::get<NextDim>(*std::min_element(m_dataVec.begin(), m_dataVec.end(), &LessCompareSpecificTupleValue<NextDim, Args...>));
	double noOfBin = std::ceil((maxValue - minValue) / binWidth) + 1;
	//having one extra because comparisons are [range), i.e., in some cases the right value can be out of bounds and not in the bin range
	//that's the case if (maxValue - minValue) / binWidth returns an almost-integral number
	countNo *= (size_t)noOfBin;
	binVectors[binVectors.size() - 2 * (sizeof...(RestDim) + 1)].reserve(noOfBin);
	//not so friendly with memory, but going with +- 0.5binWidth migth (I'm not sure) cause some super rare IEEE754 errors
	//I need precision here so no unnecessary + and - operations
	binVectors[binVectors.size() - 2 * sizeof...(RestDim) - 1].reserve(noOfBin + 1);
	for (size_t i = 0; i < (size_t)noOfBin; i++) {
		binVectors[binVectors.size() - 2 * (sizeof...(RestDim) + 1)].emplace_back(minValue + (i + 0.5) * binWidth);
		binVectors[binVectors.size() - 2 * sizeof...(RestDim) - 1].emplace_back(minValue + i * binWidth);
	}
	binVectors[binVectors.size() - 2 * sizeof...(RestDim) - 1].emplace_back(minValue + noOfBin * binWidth);
	if constexpr (sizeof...(RestDim) != 0) {
		PrepareBinVectors<RestDim...>(binVectors, binWidth, countNo);
	}
}

#if _HAS_CXX20
template<typename... Args>
requires G4_AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
template <size_t NextDim, size_t... RestDim>
void ProcessCsvData<Args...>::SortDataVecForCounting(std::vector<std::vector<double>>& binVectors, const iterator_t begin, const iterator_t end) {
	std::sort(begin, end, &LessCompareSpecificTupleValue<NextDim, Args...>);
	if constexpr (sizeof...(RestDim) != 0) {
		iterator_t nextBegin = begin;
		for (size_t i = 1; i < binVectors[binVectors.size() - 2 * sizeof...(RestDim) - 1].size(); i++) {
			/*if (binVectors.size() == 2 * (sizeof...(RestDim) + 1)) {
				iterator_t nextEnd = std::lower_bound(nextBegin, end, binVectors[binVectors.size() - 2 * sizeof...(RestDim) - 1][i],
					&LessCompareSpecificTupleWithADoubleValue<NextDim, Args...>);
				m_taskGroup.exec( [&] (iterator_t forward_nextBegin, iterator_t forward_nextEnd) {
						SortDataVecForCounting<RestDim...>(binVectors, forward_nextBegin, forward_nextEnd);
					}, nextBegin, nextEnd);
				nextBegin = nextEnd;
			}
			else {*///The commented part doesn't speed up (it's fast anyway), no need for going mutli-thread
				iterator_t nextEnd = std::lower_bound(nextBegin, end, binVectors[binVectors.size() - 2 * sizeof...(RestDim) - 1][i],
					&LessCompareSpecificTupleWithADoubleValue<NextDim, Args...>);
				SortDataVecForCounting<RestDim...>(binVectors, nextBegin, nextEnd);
				nextBegin = nextEnd;
			//}
		}
		/*if (binVectors.size() == 2 * (sizeof...(RestDim) + 1))
			m_taskGroup.wait();*/
	}
}

#if _HAS_CXX20
template<typename... Args>
requires G4_AnslysisTypes<Args...>
#else
template <typename... Args>
#endif // _HAS_CXX20
template <size_t NextDim, size_t... RestDim>
void ProcessCsvData<Args...>::CountFinalData(const std::vector<std::vector<double>>& binVectors,
								std::vector<size_t>& countVector, iterator_t begin, iterator_t end, const size_t multiplier) {
	/*
	only the last sorted dimension is used for counting here. On the other hand, multiplier passes the information
	from higher-order dimensions that helps to find the exact index where to change the counter.
	On the other hand, iterators are used to divide the vector for the recursive-like thing of variad tempaltes
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
					G4Exception("ProcessCsvData<Args...>::CountFinalData", "FE1026", FatalException, err);
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
requires G4_AnslysisTypes<Args...>
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
				//stringHelper is always empty in this case because this works only for sizeof...(Dimenstions) == 1
				if (countVector[coef] != 0) {
					try {
						//The following should work with some reasonable bin values and efficiency tables
						auto itr1 = std::lower_bound(m_efficiencyValues.begin(), m_efficiencyValues.end(), binVectors[0][i],
							[](std::pair<double, double> aPair, double value) {return aPair.first < value; });
						double efficiency = LinearInterpolate2D((itr1 - 1)->second, itr1->second, (itr1 - 1)->first, itr1->first, binVectors[0][i], 0.);
						outFile << binVectors[0][i] << ',' << efficiency * countVector[coef] << '\n';
					}
					catch (...) {
						G4Exception("ProcessCsvData<Args...>::PrintOutputData", "FE1027", FatalException,
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

endChR

#endif // !ProcessCsvData_hpp