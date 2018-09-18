
include_directories(
    /usr/local/include/libmongoc-1.0
    /usr/local/include/libbson-1.0
    /usr/local/include/mongocxx/v_noabi
)

link_directories(
        /usr/local/lib
)

set(LIB_LIST
        base_utils
        msgqueue
        jsoncpp
        pthread
        sasl2
        ssl
        crypto
        rt
        mongoc-1.0
        bson-1.0
        mysql
        bsoncxx
)

add_library(wx_tcp
    wx_tcp.h
    wx_tcp.cpp
    net_tcp.h
    net_tcp.cpp
)

target_link_libraries(wx_tcp ${LIB_LIST} )
#set_target_properties(wx PROPERTIES OUTPUT_NAME wx)
