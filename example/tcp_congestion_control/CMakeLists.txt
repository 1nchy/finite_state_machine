cmake_minimum_required(VERSION 3.26)
project(tcp_congestion_control)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE "Debug")

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../cmake)

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

set(PROJECT_SOURCE_FILE main.cpp)
file(GLOB_RECURSE PROJECT_SRC_FILE "src/*.cpp")
list(APPEND PROJECT_SOURCE_FILE ${PROJECT_SRC_FILE})
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILE})

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/../../include
)
target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

set(THIRD_LIB_NAME finite_state_machine)
find_library(third_lib_${THIRD_LIB_NAME} ${THIRD_LIB_NAME} ../../lib)
if (third_lib_${THIRD_LIB_NAME})
    target_link_libraries(${PROJECT_NAME}
        PUBLIC ${third_lib_${THIRD_LIB_NAME}}
    )
endif()