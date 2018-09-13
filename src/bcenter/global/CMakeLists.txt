
include_directories(
  /usr/local/include/libmongoc-1.0
  /usr/local/include/libbson-1.0  
    )
link_directories(/usr/local/lib)

add_library(global 

boon_global.cpp  
boon_global.h    
clock.cpp
clock.h

)


target_link_libraries(global msgqueue  jsoncpp  pthread sasl2 ssl crypto rt mongoc-1.0 bson-1.0)

set_target_properties(global PROPERTIES OUTPUT_NAME global)
