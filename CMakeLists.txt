cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_FLAGS "-std=gnu99")
set(CMAKE_C_STANDARD 99)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

file(GLOB LIBMUTANT_SOURCE  "src/*.c")
file(GLOB LIBFANCYRL_SOURCE "src/fancyrl/*.c")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_definitions(-DPARALLEL_MARK)
add_library(mutant SHARED ${LIBMUTANT_SOURCE})
target_link_libraries(mutant gc)

add_library(fancyrl SHARED ${LIBFANCYRL_SOURCE})
target_link_libraries(fancyrl mutant termcap)

add_executable(str_example ${CMAKE_SOURCE_DIR}/examples/str_example.c)
target_link_libraries(str_example mutant)

add_executable(term_example ${CMAKE_SOURCE_DIR}/examples/term_example.c)
target_link_libraries(term_example fancyrl mutant)
