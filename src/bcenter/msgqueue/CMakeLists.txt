
include_directories(
  /usr/local/include/libmongoc-1.0
  /usr/local/include/libbson-1.0  
    )
link_directories(/usr/local/lib)

add_library(msgqueue
     
boon_msgqueue.cpp  
boon_condition.cpp  
boon_msgqueue.h    
boon_condition.h    
boon_mutex.cpp     
boon_mutex.h       

)


target_link_libraries(msgqueue  pthread sasl2 ssl crypto rt mongoc-1.0 bson-1.0 )

#ADD_LIBRARY(alg SHARED ${algorithm})
set_target_properties(msgqueue PROPERTIES OUTPUT_NAME msgqueue)
