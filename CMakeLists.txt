cmake_minimum_required(VERSION 3.26)
project(finite_state_machine)

set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts" CACHE STRING "cmake cxx flags")

# set(CMAKE_SOURCE_FILE_DIRECTORY ${PROJECT_SOURCE_DIR}/src)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# file(GLOB_RECURSE SOURCE_FILE ${CMAKE_SOURCE_FILE_DIRECTORY}/*.cpp)

# add_library(${PROJECT_NAME} STATIC ${SOURCE_FILE})

# target_include_directories(${PROJECT_NAME} PUBLIC
#     ${PROJECT_SOURCE_DIR}/include
# )