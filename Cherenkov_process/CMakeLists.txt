add_library(ChR_process_lib STATIC)

target_link_libraries(ChR_process_lib PUBLIC "${Geant4_LIBRARIES}")

target_include_directories(ChR_process_lib
  PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

file(GLOB use_lib_files CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/source/*.cc" "${CMAKE_CURRENT_LIST_DIR}/include/*.hh")

target_sources(ChR_process_lib
  PRIVATE ${use_lib_files})