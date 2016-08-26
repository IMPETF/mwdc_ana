
set(includedir ${CMAKE_CURRENT_SOURCE_DIR}/include)
ROOT_GLOB_HEADERS(headers ${includedir}/*.h)
FILE(GLOB srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

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