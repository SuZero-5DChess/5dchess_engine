cmake_minimum_required(VERSION 3.12)
project(engine)

# you may need these 2 lines, and set your own paths
# set(PYTHON_EXECUTABLE "your/python/path")
# set(PYTHON_INCLUDE_DIRECTORY "your/python/include/path")

# if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANGCXX)
#     add_compile_options(-Wall -Wextra -pedantic-errors)
# endif()

# Include the src directory for header files
include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(extern/pybind11)
pybind11_add_module(engine engine.cpp board_2d.cpp)
