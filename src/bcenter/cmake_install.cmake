# Install script for directory: /home/boon/boonpark/source/bcenter

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
    set(CMAKE_INSTALL_CONFIG_NAME "")
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

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/boon/boonpark/source/bcenter/main/cmake_install.cmake")
  include("/home/boon/boonpark/source/bcenter/mongodb/cmake_install.cmake")
  include("/home/boon/boonpark/source/bcenter/msgqueue/cmake_install.cmake")
  include("/home/boon/boonpark/source/bcenter/mysql/cmake_install.cmake")
  include("/home/boon/boonpark/source/bcenter/bipc/cmake_install.cmake")
  include("/home/boon/boonpark/source/bcenter/global/cmake_install.cmake")
  include("/home/boon/boonpark/source/bcenter/in/cmake_install.cmake")
  include("/home/boon/boonpark/source/bcenter/out/cmake_install.cmake")
  include("/home/boon/boonpark/source/bcenter/bop/cmake_install.cmake")
  include("/home/boon/boonpark/source/bcenter/bvs/cmake_install.cmake")
  include("/home/boon/boonpark/source/bcenter/bgui/cmake_install.cmake")
  include("/home/boon/boonpark/source/bcenter/chewei/cmake_install.cmake")
  include("/home/boon/boonpark/source/bcenter/test_mongo/cmake_install.cmake")
  include("/home/boon/boonpark/source/bcenter/bled/cmake_install.cmake")
  include("/home/boon/boonpark/source/bcenter/wx/cmake_install.cmake")

endif()

