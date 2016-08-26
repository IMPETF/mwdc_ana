cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(mwdc)

#---------find ROOT------------------------

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT 5.34 REQUIRED CONFIG COMPONENTS Eve RGL Geom)

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

#----------- Modules---------------------

add_subdirectory(kernel)
add_subdirectory(eve)

#--------scripts installation------------

# rootlogon.C or other scripts which have cmake related variables in the text
# are copied using configure_file command. These variables need to be 
# substitued during the configuration stage, i.e. re-running # cmake each time # it changes.
install(CODE "MESSAGE(\"Installing scripts:\")")
set(SCRIPTS rootlogon.C)
foreach(_script ${SCRIPTS})
  configure_file(
    scripts/${_script}
    ${CMAKE_INSTALL_PREFIX}/${_script}   
    )
endforeach()

# As configure_file command needs to re-run cmake again after _script changes,
# I decided to use install command as follows to copy other scripts, make the # development work much productive. In the following way, the _script can be  # updated every time make install is called without re-running cmake every time.
FILE(GLOB scripts ${CMAKE_SOURCE_DIR}/scripts/*)
foreach(_script ${SCRIPTS})
	list(REMOVE_ITEM scripts ${CMAKE_SOURCE_DIR}/scripts/${_script})
endforeach()
install(CODE "MESSAGE(\"Installing programs:\")")
foreach(_script ${scripts})
	install(PROGRAMS ${_script} DESTINATION ./)
endforeach()

#-----------macros installation---------------

install(CODE "MESSAGE(\"Installing Macros:\")")
FILE(GLOB macros ${CMAKE_SOURCE_DIR}/macros/*.C)
foreach(_macro ${macros})
  install(FILES ${_macro} DESTINATION ./)
endforeach()

#add_custom_target(basic DEPENDS kernel scripts)
