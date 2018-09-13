cmake_minimum_required(VERSION 2.8)

PROJECT(UTILS)

include_directories(
        ${SOLUTION_INCLUDE_DIRS}
        ../include
)

set(SRC_LIST utils.cpp )

add_library(base_utils SHARED ${SRC_LIST})

#add_executable(unit_util unit_main.cpp)
#target_link_libraries(unit_util base_utils)
