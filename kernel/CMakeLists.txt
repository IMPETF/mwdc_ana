# cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
# project(kernel)

# add_definitions(-std=c++11)
# add_compile_options(-std=c++11)

set(includedir ${CMAKE_CURRENT_SOURCE_DIR}/include)
ROOT_GLOB_HEADERS(headers ${includedir}/*.h)
# foreach(h ${headers})
#   if(h MATCHES TrackFit)
#     list(REMOVE_ITEM headers ${h})
#   endif()
# endforeach()
FILE(GLOB srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

MESSAGE(STATUS "${headers}")
ROOT_GENERATE_DICTIONARY(kernel_Dict ${headers} LINKDEF ${includedir}/kernel_LinkDef.h OPTIONS -p)

add_library(mwdc_kernel SHARED ${headers} ${srcs}
			kernel_Dict.cxx)
target_link_libraries(mwdc_kernel ${ROOT_LIBRARIES})
target_include_directories(mwdc_kernel PUBLIC ${includedir})
# get_target_property(incdirs mwdc_kernel INCLUDE_DIRECTORIES)
# MESSAGE(STATUS "incdirs: ${incdirs}")

#-------------------------------------------------------
install(TARGETS mwdc_kernel DESTINATION lib)
foreach(_header ${headers})
  install(FILES ${_header} DESTINATION include)
endforeach()