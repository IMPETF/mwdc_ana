
set(includedir ${CMAKE_CURRENT_SOURCE_DIR}/include)
ROOT_GLOB_HEADERS(headers ${includedir}/*.h)
FILE(GLOB srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

#MESSAGE(STATUS "${headers}")
get_target_property(incdir mwdc_kernel INCLUDE_DIRECTORIES)
set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES "${incdir}")
# get_directory_property(newincdir INCLUDE_DIRECTORIES)
# message(STATUS "eve includedir: ${incdir}")
# message(STATUS "eve newincludedir: ${newincdir}")
#set(rootcint_incdir)
#foreach(d ${incdir})
#    set(rootcint_incdir ${rootcint_incdir} -I${d})
#endforeach()

ROOT_GENERATE_DICTIONARY(eve_Dict ${headers} LINKDEF ${includedir}/eve_LinkDef.h OPTIONS -p)

add_library(mwdc_eve SHARED ${headers} ${srcs}
			eve_Dict.cxx)
target_link_libraries(mwdc_eve mwdc_kernel ${ROOT_LIBRARIES})
target_include_directories(mwdc_eve PUBLIC ${includedir})

#-------------------------------------------------------
install(TARGETS mwdc_eve DESTINATION lib)

foreach(_header ${headers})
  install(FILES ${_header} DESTINATION include)
endforeach()

FILE(GLOB geom_files ${CMAKE_CURRENT_SOURCE_DIR}/test/*.root)
foreach(file ${geom_files})
	install(FILES ${file} DESTINATION geom)
endforeach()

#---------------------------------------------------------
add_executable(evetest test/main.cxx)
target_link_libraries(evetest mwdc_kernel mwdc_eve)

add_executable(fit_debug test/fit_debug.cxx)
target_link_libraries(fit_debug mwdc_kernel mwdc_eve)
install(TARGETS evetest fit_debug DESTINATION lib)
