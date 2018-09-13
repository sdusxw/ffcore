cmake_minimum_required(VERSION 2.8)

include_directories(
        ${SOLUTION_INCLUDE_DIRS}
        ../base_poll
        ../utils
)

set(SRC_LIST
        base_msg.cpp
        base_msg_cli.cpp
        base_cb_cli.cpp
        base_socketapi.cpp
        base_oscore.cpp
        base_tcp.cpp
        base_udp.cpp
        )

set(LIB_LIST
        base_poll
        )

add_library(base_socketapi SHARED ${SRC_LIST})
target_link_libraries(base_socketapi ${LIB_LIST})

#add_executable(unit_udp_server unit_udp_server.cpp)
#target_link_libraries(unit_udp_server base_socketapi base_utils )

#add_executable(unit_udp_client unit_udp_client.cpp)
#target_link_libraries(unit_udp_client base_socketapi base_utils)