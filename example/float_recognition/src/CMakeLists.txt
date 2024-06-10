file(GLOB_RECURSE SOURCE_FILE "*.cpp")

add_library(${PROJECT_SOURCE_TARGET} ${SOURCE_FILE})

target_include_directories(${PROJECT_SOURCE_TARGET}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)