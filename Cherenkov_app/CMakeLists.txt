add_executable(ChR_app_exe)

target_link_libraries(ChR_app_exe PRIVATE ChR_process_lib)

target_include_directories(ChR_app_exe
	PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

file(GLOB use_exe_files CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/source/*.cc" "${CMAKE_CURRENT_LIST_DIR}/include/*.hh")

target_sources(ChR_app_exe
	PRIVATE theMain.cpp ${use_exe_files})