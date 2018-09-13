
include_directories(
  /usr/local/include/libmongoc-1.0
  /usr/local/include/libbson-1.0  
    )
link_directories(/usr/local/lib)

add_library(bipc_lib

boon_bipc.cpp  
boon_bipc.h    

)


target_link_libraries(bipc_lib msgqueue  jsoncpp  pthread sasl2 ssl crypto rt mongoc-1.0 bson-1.0 mysql mongodb )
#target_link_libraries(work  pthread  curl)
#ADD_LIBRARY(alg SHARED ${algorithm})
#set_target_properties(bipc_lib PROPERTIES OUTPUT_NAME bipc_lib)
