cmake_minimum_required(VERSION 3.12)
project(engine)

# you may need these 2 lines, and set your own paths
# set(PYTHON_EXECUTABLE "your/python/path")
# set(PYTHON_INCLUDE_DIRECTORY "your/python/include/path")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANGCXX)
#     add_compile_options(-Wall -Wextra -pedantic-errors)
# endif()

# Set up options
option(TEST "Build tests" OFF) # Default is OFF

# Include the src directory for header files
include_directories(${CMAKE_SOURCE_DIR})

set(COMMON_SOURCE_FILES
    board.cpp
    multiverse.cpp
)

message("By default this creates a python library. Run with -DTEST=ON to build the tests.")

if(TEST)
    message(STATUS "Compiling test.cpp...")
    add_executable(test_exec test.cpp ${COMMON_SOURCE_FILES})
else()
    message(STATUS "Building python library...")
    add_subdirectory(extern/pybind11)
    pybind11_add_module(engine bind.cpp ${COMMON_SOURCE_FILES})
endif()

