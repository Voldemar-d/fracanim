# CMakeList.txt : CMake project for fracanim, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (fracanim "fracanim.cpp" "fracanim.h" "pngwrite.h" "fracdraw.h" "fracdraw.cpp" "inputparser.h" "pngwrite.cpp")

find_package(PNG REQUIRED)

if(WIN32)
find_path(PNG_INCLUDE_DIR PNG)
target_include_directories(${PROJECT_NAME} PRIVATE ${PNG_INCLUDE_DIR})
endif(WIN32)

target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
