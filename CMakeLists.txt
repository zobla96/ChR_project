### TESTED ON VS 2022 (MSVC)
### AND CMAKE 3.23.0 and 3.29.3

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(STATUS "You are trying to make your build in the source dir.")
  message(STATUS "Don't do that. While building the project, make sure")
  message(STATUS "to separate the build and source folders!")
  message(STATUS "Feel free to delete the created CMakeFiles/ and CMakeCache.txt")
  message(FATAL_ERROR "TRYING TO BUILD THE PROJECT IN THE SOURCE DIRECTORY!")
endif()

cmake_minimum_required(VERSION 3.16...3.29 FATAL_ERROR)
#NOTE: I think 3.16 should be able to run, but have no tested it!

project(ChR_project
  VERSION 1.0.1
  DESCRIPTION "An example on how to use Cherenkov processes"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED 17)
option(USE_MY_CONFIGURATION "To use non-default targets' settings and flags" ON)

set(THE_DEFAULT_CXX_FLAGS "" CACHE STRING "The backup of default flags")

# a function to speed up the compilation process (generate object files with multiple threads)
function(use_multithread_compile mtFlag)
  if(THE_DEFAULT_CXX_FLAGS STREQUAL "")
    set(THE_DEFAULT_CXX_FLAGS ${CMAKE_CXX_FLAGS} CACHE INTERNAL "Flags not shown in GUI - defaults not modifiable")
  endif()
  set(CMAKE_CXX_FLAGS "${THE_DEFAULT_CXX_FLAGS} ${mtFlag}" CACHE STRING "Setting user non-default flags" FORCE)
  message(STATUS ${CMAKE_CXX_FLAGS})
endfunction()

if(USE_MY_CONFIGURATION)
  set(CMAKE_CXX_STANDARD 20)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    use_multithread_compile("/MP")
  ### for GNU and Clang, not sure about other compilers
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    include(ProcessorCount)
    ProcessorCount(N_CORES)
    if(NOT N_CORES EQUAL 0)
      use_multithread_compile("-j${N_CORES}")
    else()
      message(WARNING "Failed to determine the number of processors. Compiling with a single thraed")
    endif()
  endif()

else() ###(!USE_MY_CONFIGURATION)
  set(CMAKE_CXX_STANDARD 17)
  if(NOT THE_DEFAULT_CXX_FLAGS STREQUAL "")
    set(CMAKE_CXX_FLAGS "${THE_DEFAULT_CXX_FLAGS}" CACHE STRING "Flags used by the CXX compiler during all build types." FORCE)
    set(THE_DEFAULT_CXX_FLAGS "" CACHE INTERNAL "The backup of default flags")
  endif() ###(end of THE_DEFAULT_CXX_FLAGS)

endif() ###(end of USE_MY_CONFIGURATION)

option(USE_G4_VISUALIZATION "Use Geant4 ui and vis" ON)
if(USE_G4_VISUALIZATION)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

include(${Geant4_USE_FILE})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")

set(filesToCopy "initialization.mac;run1.mac;run2.mac;vis.mac;refractive_index_data.txt;efficiency.csv")

foreach(size_t_i ${filesToCopy})
  if(NOT EXISTS ${CMAKE_SOURCE_DIR}/${size_t_i})
    message(WARNING "File \"${size_t_i}\" not found and not copied!")
    continue()
  endif()
  configure_file(
    ${CMAKE_SOURCE_DIR}/${size_t_i}
    ${CMAKE_BINARY_DIR}/${size_t_i}
    COPYONLY
  )
endforeach()

add_subdirectory(Cherenkov_process)
add_subdirectory(Cherenkov_app)

### one may add Install if needed