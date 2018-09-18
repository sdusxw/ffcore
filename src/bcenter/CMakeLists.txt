# specify the source directory and the binary directory
# press configure
# set CMAKE_INSTALL_PREFIX to the path where you want to install the program
# press configure
# check BUILD_WITH_QT if you want to build the program with a QT-Config GUI
# check GENERATE_DEB_PACKAGE if you want to build a debian package (only on Linux)
#
# UNIX Makefile:
# 1) go to the binary folder and type "make" to build the project
# 2) (optional) type "make install all" to install the files into the install
#    directory
# 3) (optional) type "make package" to create a package of the install folder
#    (.tgz file if GENERATE_DEB_PACKAGE=false, .deb file if GENERATE_DEB_PACKAGE=true)
#
# Microsoft Visual C++:
# 1) open the .sln file
# 2) change the mode to "Release" -> only Release is supported!
# 3) build the project "ALL_BUILD" to build the opentld project
# 4) build the project "INSTALL" to install the files into the install
#    directory
# 5) build the project "PACKAGE" to create an NSIS-installer (NSIS is required)
#
# for Eclipse CDT
# cmake -G "Eclipse CDT4 - Unix Makefiles" ../src

cmake_minimum_required(VERSION 2.8)

project(BCENTER)

set(CMAKE_USE_RELATIVE_PATHS ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

#find_package(OpenCV REQUIRED)

if(APPLE)
	add_definitions(-DHAVE_XLOCALE_H)
endif(APPLE)

include_directories(
		../include
)

set(SOLUTION_INCLUDE_DIRS
    ${src_dir}/main
)

#-------------------------------------------------------------------------------
#add subdirectories
add_subdirectory(main)
add_subdirectory(mongodb)
add_subdirectory(msgqueue)
add_subdirectory(mysql)
add_subdirectory(bipc)
add_subdirectory(global)
add_subdirectory(in)
add_subdirectory(out)
add_subdirectory(bop)
add_subdirectory(bvs)
add_subdirectory(bgui)
add_subdirectory(chewei)
add_subdirectory(test_mongo)
add_subdirectory(bled)
add_subdirectory(wx)
add_subdirectory(wx_tcp)
