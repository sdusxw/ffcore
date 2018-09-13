cmake_minimum_required(VERSION 2.8)

include_directories(
        ${SOLUTION_INCLUDE_DIRS}
        ../include
        ../utils
        ../network/base_socketapi
        ../network/base_poll
        ../network/mdbindex
)

set(SRC_LIST_QUEUE safeguard_queue.cpp)

add_library(base_safequeue SHARED ${SRC_LIST_QUEUE})

#add_executable(unit_queue unit_queue.cpp)
#add_executable(unit_client unit_client.cpp my_client.cpp)
#add_executable(unit_server my_client.cpp my_server.cpp unit_server.cpp)
#add_executable(unit_mulmap unit_multimap.cpp)
#add_executable(unit_rapidjson unit_rapidjson.cpp)

#target_link_libraries(unit_queue tsafeguard_queue)
#target_link_libraries(unit_client base_poll base_socketapi tutils)
#target_link_libraries(unit_server base_poll base_socketapi tutils)
#target_link_libraries(unit_rapidjson base_utils)
