cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(kernel)

find_package(ROOT REQUIRED)
#include(ROOTConfig.cmake)
include(RootNewMacros.cmake)

add_definitions(-std=c++11)
#add_compile_options(-std=c++11)

include_directories(${ROOT_INCLUDE_DIRS})

FILE(GLOB headers ${PROJECT_SOURCE_DIR}/*.h)

ROOT_GENERATE_DICTIONARY(global_Dict ${CMAKE_CURRENT_SOURCE_DIR}/global.h LINKDEF global_LinkDef.h)
ROOT_GENERATE_DICTIONARY(BoardInfo_Dict ${CMAKE_CURRENT_SOURCE_DIR}/BoardInfo.h LINKDEF BoardInfo_LinkDef.h)
ROOT_GENERATE_DICTIONARY(CrateInfo_Dict ${CMAKE_CURRENT_SOURCE_DIR}/CrateInfo.h LINKDEF CrateInfo_LinkDef.h)
ROOT_GENERATE_DICTIONARY(utility_Dict ${CMAKE_CURRENT_SOURCE_DIR}/utility.h LINKDEF utility_LinkDef.h)
ROOT_GENERATE_DICTIONARY(GeometryInfo_Dict ${CMAKE_CURRENT_SOURCE_DIR}/GeometryInfo.h LINKDEF GeometryInfo_LinkDef.h)
ROOT_LINKER_LIBRARY(mwdc global.h global_Dict.cxx BoardInfo.h BoardInfo_Dict.cxx 
		    CrateInfo.h CrateInfo_Dict.cxx
		    jsoncpp.cpp
		    utility_Dict.cxx utility.cxx
		    GeometryInfo_Dict.cxx GeometryInfo.cxx
		    LIBRARIES Core Cint RIO Hist Gpad Tree Rint MathCore)
		    
#-------------------------------------------------------
install(TARGETS mwdc DESTINATION lib)
foreach(_header ${headers})
  install(FILES ${_header} DESTINATION include)
endforeach()