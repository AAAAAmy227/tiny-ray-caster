# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(PolyTrcProject)

# Set the C++ standard version
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable target, specifying the source file(s)
add_executable(PolyTrc poly-trc.cpp)

# If you have additional dependencies or include directories, you can specify them here.
# For example, if your header files are in a different directory:
# include_directories(${PROJECT_SOURCE_DIR}/include)


