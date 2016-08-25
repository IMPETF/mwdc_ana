cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(kernel)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT 5.34 REQUIRED CONFIG)

if(NOT ROOT_FOUND)
	MESSAGE(FATAL_ERROR "ROOT not found")
else()
	MESSAGE(STATUS "ROOT_ENV: $ENV{ROOTSYS}")
	MESSAGE(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
	MESSAGE(STATUS "ROOTSYS: ${ROOTSYS}")
	MESSAGE(STATUS "ROOT_FOUND: ${ROOT_FOUND}")
	MESSAGE(STATUS "ROOT_VERSION: ${ROOT_VERSION}")
	MESSAGE(STATUS "ROOT_INCLUDE_DIR: ${ROOT_INCLUDE_DIR}")
	MESSAGE(STATUS "ROOT_LIBRARY_DIR: ${ROOT_LIBRARY_DIR}")
	MESSAGE(STATUS "ROOT_LIBRARIES: ${ROOT_LIBRARIES}")
	MESSAGE(STATUS "ROOT_DEFINITIONS: ${ROOT_DEFINITIONS}")
	MESSAGE(STATUS "ROOT_USE_FILE: ${ROOT_USE_FILE}")
endif()

include(${ROOT_USE_FILE})
# include_directories(${ROOT_INCLUDE_DIRS})

# add_definitions(-std=c++11)
# add_compile_options(-std=c++11)

FILE(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)

ROOT_GENERATE_DICTIONARY(global_Dict ${CMAKE_CURRENT_SOURCE_DIR}/global.h LINKDEF global_LinkDef.h OPTIONS -p)
ROOT_GENERATE_DICTIONARY(BoardInfo_Dict ${CMAKE_CURRENT_SOURCE_DIR}/BoardInfo.h LINKDEF BoardInfo_LinkDef.h OPTIONS -p)
ROOT_GENERATE_DICTIONARY(CrateInfo_Dict ${CMAKE_CURRENT_SOURCE_DIR}/CrateInfo.h LINKDEF CrateInfo_LinkDef.h OPTIONS -p)
ROOT_GENERATE_DICTIONARY(utility_Dict  ${CMAKE_CURRENT_SOURCE_DIR}/utility.h LINKDEF utility_LinkDef.h OPTIONS -p)
ROOT_GENERATE_DICTIONARY(GeometryInfo_Dict ${CMAKE_CURRENT_SOURCE_DIR}/GeometryInfo.h LINKDEF GeometryInfo_LinkDef.h OPTIONS -p)
ROOT_GENERATE_DICTIONARY(DriftInfo_Dict ${CMAKE_CURRENT_SOURCE_DIR}/DriftInfo.h LINKDEF DriftInfo_LinkDef.h OPTIONS -p)
# ROOT_LINKER_LIBRARY(mwdc global.h global_Dict.cxx BoardInfo.h BoardInfo_Dict.cxx 
# 		    CrateInfo.h CrateInfo_Dict.cxx
# 		    jsoncpp.cpp
# 		    utility_Dict.cxx utility.cxx
# 		    GeometryInfo_Dict.cxx GeometryInfo.cxx
# 		    DriftInfo_Dict.cxx DriftInfo.cxx
# 		    TrackFit.h
# 		    LIBRARIES Core Cint RIO Hist Gpad Tree Rint MathCore)

add_library(mwdc SHARED global.h global_Dict.cxx BoardInfo.h BoardInfo_Dict.cxx 
			CrateInfo_Dict.cxx CrateInfo.h 
			jsoncpp.cpp 
			utility.cxx utility_Dict.cxx
			GeometryInfo.cxx GeometryInfo_Dict.cxx 
			DriftInfo.cxx DriftInfo_Dict.cxx 
			TrackFit.h)
target_link_libraries(mwdc ${ROOT_LIBRARIES})

#-------------------------------------------------------
install(TARGETS mwdc DESTINATION lib)
foreach(_header ${headers})
  install(FILES ${_header} DESTINATION include)
endforeach()