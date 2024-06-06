# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("fracanim")

set(CMAKE_CXX_STANDARD 20)

# Include sub-projects.
add_subdirectory ("fracanim")

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
