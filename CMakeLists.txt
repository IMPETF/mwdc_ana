cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(mwdc_ana)

add_subdirectory(kernel)
# add_subdirectory(eve)

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
	list(REMOVE_ITEM scripts ${CMAKE_SOURCE_DIR}/scripts/_script)
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
