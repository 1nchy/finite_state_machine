cmake_minimum_required(VERSION 3.26)
project(finite_state_machine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE "Release")
# set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_SOURCE_FILE_DIRECTORY ${PROJECT_SOURCE_DIR}/src)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

macro(add_third_libraries lib_name)
    message(STATUS "add third library: " ${lib_name})
    find_package(${lib_name} REQUIRED MODULE)
    if (${lib_name}_FOUND)
        if (${lib_name}_INCLUDE_DIR)
            include_directories(${${lib_name}_INCLUDE_DIR})
        endif (${lib_name}_INCLUDE_DIR)
        if (${lib_name}_LIBRARIES)
            link_libraries(${${lib_name}_LIBRARIES})
        endif (${lib_name}_LIBRARIES)
    endif (${lib_name}_FOUND)
endmacro(add_third_libraries)

# add_third_libraries(<third_lib>)

# file(GLOB_RECURSE LIB_SOURCE_FILE ${CMAKE_SOURCE_FILE_DIRECTORY}/*.cpp)

# add_library(${PROJECT_NAME} STATIC ${LIB_SOURCE_FILE})

# target_include_directories(${PROJECT_NAME} PUBLIC
#     ${PROJECT_SOURCE_DIR}/include
# )

include(CTest)
add_subdirectory(test)