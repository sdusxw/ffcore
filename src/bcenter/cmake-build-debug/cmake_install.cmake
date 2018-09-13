# Install script for directory: /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/main/cmake_install.cmake")
  include("/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/mongodb/cmake_install.cmake")
  include("/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/msgqueue/cmake_install.cmake")
  include("/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/mysql/cmake_install.cmake")
  include("/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/bipc/cmake_install.cmake")
  include("/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/global/cmake_install.cmake")
  include("/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/in/cmake_install.cmake")
  include("/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/out/cmake_install.cmake")
  include("/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/bop/cmake_install.cmake")
  include("/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/bvs/cmake_install.cmake")
  include("/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/bgui/cmake_install.cmake")
  include("/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/chewei/cmake_install.cmake")
  include("/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/test_mongo/cmake_install.cmake")
  include("/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/bled/cmake_install.cmake")
  include("/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/wx/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
