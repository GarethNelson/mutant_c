cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

file(GLOB LIBMUTANT_SOURCE  "src/*.c")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_definitions(-DPARALLEL_MARK)
add_library(mutant SHARED ${LIBMUTANT_SOURCE})
target_link_libraries(mutant gc)

add_executable(str_example ${CMAKE_SOURCE_DIR}/examples/str_example.c)
target_link_libraries(str_example mutant)

