# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Produce verbose output by default.
VERBOSE = 1

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/tcorberes/Desktop/sfe/quadruped-walkgen

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/tcorberes/Desktop/sfe/quadruped-walkgen/build_

# Utility rule file for benchmarks-cpp-quadruped.

# Include the progress variables for this target.
include benchmark/CMakeFiles/benchmarks-cpp-quadruped.dir/progress.make

benchmark/CMakeFiles/benchmarks-cpp-quadruped:
	cd /home/tcorberes/Desktop/sfe/quadruped-walkgen/build_/benchmark && ./quadruped ${INPUT}

benchmarks-cpp-quadruped: benchmark/CMakeFiles/benchmarks-cpp-quadruped
benchmarks-cpp-quadruped: benchmark/CMakeFiles/benchmarks-cpp-quadruped.dir/build.make

.PHONY : benchmarks-cpp-quadruped

# Rule to build all files generated by this target.
benchmark/CMakeFiles/benchmarks-cpp-quadruped.dir/build: benchmarks-cpp-quadruped

.PHONY : benchmark/CMakeFiles/benchmarks-cpp-quadruped.dir/build

benchmark/CMakeFiles/benchmarks-cpp-quadruped.dir/clean:
	cd /home/tcorberes/Desktop/sfe/quadruped-walkgen/build_/benchmark && $(CMAKE_COMMAND) -P CMakeFiles/benchmarks-cpp-quadruped.dir/cmake_clean.cmake
.PHONY : benchmark/CMakeFiles/benchmarks-cpp-quadruped.dir/clean

benchmark/CMakeFiles/benchmarks-cpp-quadruped.dir/depend:
	cd /home/tcorberes/Desktop/sfe/quadruped-walkgen/build_ && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/tcorberes/Desktop/sfe/quadruped-walkgen /home/tcorberes/Desktop/sfe/quadruped-walkgen/benchmark /home/tcorberes/Desktop/sfe/quadruped-walkgen/build_ /home/tcorberes/Desktop/sfe/quadruped-walkgen/build_/benchmark /home/tcorberes/Desktop/sfe/quadruped-walkgen/build_/benchmark/CMakeFiles/benchmarks-cpp-quadruped.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : benchmark/CMakeFiles/benchmarks-cpp-quadruped.dir/depend

