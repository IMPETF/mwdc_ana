cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(mwdc_ana)

add_subdirectory(kernel)
#add_subdirectory(scripts)

set(SCRIPTS
  maketemplate
  rootlogon.C
  )

foreach(_script ${SCRIPTS})
  configure_file(
    scripts/${_script}
    ${CMAKE_INSTALL_PREFIX}/${_script}   
    )
endforeach()

add_custom_target(basic DEPENDS kernel scripts)
