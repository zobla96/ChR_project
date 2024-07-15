#Cmake version that's required
cmake_minimum_required(VERSION 3.16)

#The name of the project in build folder
project(ChR_project)

#Activating Geant_visual
option(Geant_visual "Activating Geant4 visualization" ON)
if(Geant_visual) ##This will be done if the previous function managed to activate Geant_visual
	find_package(Geant4 REQUIRED vis_all ui_all)
else()
	find_package(Geant4 REQUIRED)
endif()

#Loading of Geant files and the project folders with files .cpp and .hh
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/my_headers)
include_directories(${PROJECT_SOURCE_DIR}/my_cpps)

#Loading paths to be coppied from my_headers and my_cpps into the build folder
file(GLOB copy_cpps ${PROJECT_SOURCE_DIR}/my_cpps/*cpp)
file(GLOB copy_headers ${PROJECT_SOURCE_DIR}/my_headers/*hpp)

#Add .exe file and include paths to main function .cpp and folders with .cpp and .hh files
add_executable(executeChR_project projectCppFile.cpp ${copy_cpps} ${copy_headers})
#Add path to the Geant4 libraries
target_link_libraries(executeChR_project ${Geant4_LIBRARIES})

#Set (bound) macro files to a spectific name (myMacroList)
set(myMacroList
efficiency.csv
initialization.mac
run1.mac
run2.mac
vis.mac
refractive_index_data.txt
)

#Go through macro files in myMacroList and copy them into the build folder
foreach(int_i ${myMacroList})
	configure_file(
		${PROJECT_SOURCE_DIR}/${int_i}
		${PROJECT_BINARY_DIR}/${int_i}
		COPYONLY
	)
endforeach()

#For updating after build (DEPENDS is updated in v3.16)
add_custom_target(ChR_project DEPENDS executeChR_project)
