
include_directories(
  /usr/local/include/libmongoc-1.0
  /usr/local/include/libbson-1.0  
  /usr/include/mysql/
    )

link_directories(
    /usr/local/lib
    /usr/lib/x86_64-linux-gnu
)

add_library(mysql 

boon_mysql.cpp  
boon_mysql.h    

)


target_link_libraries(mysql msgqueue  jsoncpp  pthread sasl2 ssl crypto rt mongoc-1.0 bson-1.0  mysqlclient )
#target_link_libraries(work  pthread  curl)
#ADD_LIBRARY(alg SHARED ${algorithm})
set_target_properties(mysql PROPERTIES OUTPUT_NAME mysql)
