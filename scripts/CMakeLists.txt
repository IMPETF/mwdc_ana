cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(scripts)

set(SCRIPTS
  maketemplate
  rootlogon.C
  )

foreach(_script ${SCRIPTS})
  configure_file(
    ${_script}
    ${_script}    
    )
endforeach()