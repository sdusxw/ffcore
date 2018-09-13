cmake_minimum_required(VERSION 2.8)

project(ffcore)

cmake_policy(SET CMP0053 OLD)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g")

#
set(CMAKE_USE_RELATIVE_PATHS ON)

find_package(OpenCV REQUIRED)

#
set(src_dir "@PROJECT_SOURCE_DIR@")
set(bin_dir "@PROJECT_BINARY_DIR@")
set(lib_dir "@PROJECT_BINARY_DIR@")

SET(EXECUTABLE_OUTPUT_PATH ${bin_dir}/bin)
SET(LIBRARY_OUTPUT_PATH ${lib_dir}/lib)

set(SOLUTION_INCLUDE_DIRS
        ${src_dir}/include
        ${src_dir}/include/3rd
        ${src_dir}/include/3rd/caffe
        ${src_dir}/common
        ${src_dir}/utils
        )

# add subdirectories
#bvs 需要在32位机器上编译
if(USE_32BITS)
    add_subdirectory(bvs)
    #led屏控制
    add_subdirectory(bcontrol)
else()
#   add_subdirectory(network)
#    add_subdirectory(utils)
#    add_subdirectory(common)
    #微信支付客户端
#    add_subdirectory(oncallclient)
    #微信支付服务端
#    add_subdirectory(oncallserver)


#    add_subdirectory(web)
    add_subdirectory(bcenter)
#    add_subdirectory(bvszs)
    #旧的程序
    #add_subdirectory(bipc)
    #臻识智能相机
#    add_subdirectory(bipczs)
    #相机白名单同步
#    add_subdirectory(bipcsync)
    #中纬相机
#    add_subdirectory(bipczw)
#    add_subdirectory(bcrypto)
#    add_subdirectory(csync_server)
#    add_subdirectory(csync_client)

    #烽火服务端
   # add_subdirectory(oncallserver_FH)
endif(USE_32BITS)
