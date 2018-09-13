cmake_minimum_required(VERSION 2.8)

#ADD_DEFINITIONS(-DBASE_POLL_USE_SELECT)

include_directories(
        ${SOLUTION_INCLUDE_DIRS}
)

set(SRC_LIST
        base_poll.cpp
        bselect.cpp
        )

add_library(base_poll SHARED ${SRC_LIST})