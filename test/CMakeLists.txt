cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(THIRD_LIB_NAME ${PROJECT_NAME})
find_library(third_lib_${THIRD_LIB_NAME} ${THIRD_LIB_NAME} ../lib)
if (third_lib_${THIRD_LIB_NAME})
    link_libraries(${third_lib_${THIRD_LIB_NAME}})
endif()

macro(icy_add_test case_name)
    set(case_file ${case_name}.cpp)
    set(case_exe ${case_name}_executable)
    add_executable(${case_exe} ${case_file})
    add_test(${case_name} ${case_exe})
endmacro(icy_add_test)

icy_add_test(float_recognition)