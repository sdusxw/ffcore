
include_directories(
  /usr/local/include/libmongoc-1.0
  /usr/local/include/libbson-1.0  
    )
link_directories(/usr/local/lib)

add_library(out 

boon_out.cpp  
boon_out.h    

)


target_link_libraries(out msgqueue  jsoncpp  pthread sasl2 ssl crypto rt mongoc-1.0 bson-1.0 mysql mongodb global)

#target_link_libraries(work  pthread  curl)
#ADD_LIBRARY(alg SHARED ${algorithm})
set_target_properties(out PROPERTIES OUTPUT_NAME out)
