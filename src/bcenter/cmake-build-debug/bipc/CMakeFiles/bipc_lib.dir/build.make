# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.8

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/clion-2017.2.2/bin/cmake/bin/cmake

# The command to remove a file.
RM = /opt/clion-2017.2.2/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug

# Include any dependencies generated for this target.
include bipc/CMakeFiles/bipc_lib.dir/depend.make

# Include the progress variables for this target.
include bipc/CMakeFiles/bipc_lib.dir/progress.make

# Include the compile flags for this target's objects.
include bipc/CMakeFiles/bipc_lib.dir/flags.make

bipc/CMakeFiles/bipc_lib.dir/boon_bipc.cpp.o: bipc/CMakeFiles/bipc_lib.dir/flags.make
bipc/CMakeFiles/bipc_lib.dir/boon_bipc.cpp.o: ../bipc/boon_bipc.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object bipc/CMakeFiles/bipc_lib.dir/boon_bipc.cpp.o"
	cd /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/bipc && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bipc_lib.dir/boon_bipc.cpp.o -c /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/bipc/boon_bipc.cpp

bipc/CMakeFiles/bipc_lib.dir/boon_bipc.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bipc_lib.dir/boon_bipc.cpp.i"
	cd /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/bipc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/bipc/boon_bipc.cpp > CMakeFiles/bipc_lib.dir/boon_bipc.cpp.i

bipc/CMakeFiles/bipc_lib.dir/boon_bipc.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bipc_lib.dir/boon_bipc.cpp.s"
	cd /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/bipc && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/bipc/boon_bipc.cpp -o CMakeFiles/bipc_lib.dir/boon_bipc.cpp.s

bipc/CMakeFiles/bipc_lib.dir/boon_bipc.cpp.o.requires:

.PHONY : bipc/CMakeFiles/bipc_lib.dir/boon_bipc.cpp.o.requires

bipc/CMakeFiles/bipc_lib.dir/boon_bipc.cpp.o.provides: bipc/CMakeFiles/bipc_lib.dir/boon_bipc.cpp.o.requires
	$(MAKE) -f bipc/CMakeFiles/bipc_lib.dir/build.make bipc/CMakeFiles/bipc_lib.dir/boon_bipc.cpp.o.provides.build
.PHONY : bipc/CMakeFiles/bipc_lib.dir/boon_bipc.cpp.o.provides

bipc/CMakeFiles/bipc_lib.dir/boon_bipc.cpp.o.provides.build: bipc/CMakeFiles/bipc_lib.dir/boon_bipc.cpp.o


# Object files for target bipc_lib
bipc_lib_OBJECTS = \
"CMakeFiles/bipc_lib.dir/boon_bipc.cpp.o"

# External object files for target bipc_lib
bipc_lib_EXTERNAL_OBJECTS =

bipc/libbipc_lib.a: bipc/CMakeFiles/bipc_lib.dir/boon_bipc.cpp.o
bipc/libbipc_lib.a: bipc/CMakeFiles/bipc_lib.dir/build.make
bipc/libbipc_lib.a: bipc/CMakeFiles/bipc_lib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libbipc_lib.a"
	cd /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/bipc && $(CMAKE_COMMAND) -P CMakeFiles/bipc_lib.dir/cmake_clean_target.cmake
	cd /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/bipc && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/bipc_lib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
bipc/CMakeFiles/bipc_lib.dir/build: bipc/libbipc_lib.a

.PHONY : bipc/CMakeFiles/bipc_lib.dir/build

bipc/CMakeFiles/bipc_lib.dir/requires: bipc/CMakeFiles/bipc_lib.dir/boon_bipc.cpp.o.requires

.PHONY : bipc/CMakeFiles/bipc_lib.dir/requires

bipc/CMakeFiles/bipc_lib.dir/clean:
	cd /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/bipc && $(CMAKE_COMMAND) -P CMakeFiles/bipc_lib.dir/cmake_clean.cmake
.PHONY : bipc/CMakeFiles/bipc_lib.dir/clean

bipc/CMakeFiles/bipc_lib.dir/depend:
	cd /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/bipc /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/bipc /home/boon/svn/svnpro/minyong/branches/bcenter/bcenter2bipc/source/bcenter/cmake-build-debug/bipc/CMakeFiles/bipc_lib.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : bipc/CMakeFiles/bipc_lib.dir/depend
