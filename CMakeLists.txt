cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(mwdc_ana)

add_subdirectory(kernel)
#add_subdirectory(scripts)

#---------------------
set(SCRIPTS
  rootlogon.C
  )

# rootlogon.C has cmake related variables in the text, which needs to be 
# substitued during the configuration stage.
install(CODE "MESSAGE(\"Installing scripts:\")")
foreach(_script ${SCRIPTS})
  configure_file(
    scripts/${_script}
    ${CMAKE_INSTALL_PREFIX}/${_script}   
    )
endforeach()

# As configure_file command needs to re-run cmake again after _script changes,
# I decided to use install command as follows to ease the development work.
# In the following way, the _script will be updated every time make install is called.
FILE(GLOB scripts ${CMAKE_SOURCE_DIR}/scripts/*)
list(REMOVE_ITEM scripts ${CMAKE_SOURCE_DIR}/scripts/rootlogon.C)
install(CODE "MESSAGE(\"Installing programs:\")")
foreach(_script ${scripts})
	install(PROGRAMS ${_script} DESTINATION ./)
endforeach()

#--------------------------
install(CODE "MESSAGE(\"Installing Macros:\")")
FILE(GLOB macros ${CMAKE_SOURCE_DIR}/macros/*.C)
foreach(_macro ${macros})
  install(FILES ${_macro} DESTINATION ./)
endforeach()

#add_custom_target(basic DEPENDS kernel scripts)
