
include_directories(
    /usr/local/include/libmongoc-1.0
    /usr/local/include/libbson-1.0
    #../wx
    )
#link_directories(/usr/local/lib)

add_executable(bcenter
    booncrypto.h
    booncrypto.cpp
    #unit_fee.cpp\
        main.cpp
)


target_link_libraries(bcenter pthread mongodb msgqueue mysql  bipc_lib global in out bop bvs_lib bgui chewei test_mongo boon_bled  wx jsoncpp)

set_target_properties(bcenter PROPERTIES OUTPUT_NAME bcenter)
