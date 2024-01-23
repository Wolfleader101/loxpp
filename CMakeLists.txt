cmake_minimum_required (VERSION 3.20)

SET(PROJECT_NAME Loxpp)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Compiler-specific flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    add_compile_options(
        -Weverything -fcolor-diagnostics
        -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded
        -Wno-deprecated-declarations -Wno-exit-time-destructors
        -Wno-switch-enum -Wno-weak-vtables -Wno-global-constructors
        -Wno-newline-eof
    )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Wpedantic -fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options(/W4)
endif()


file(GLOB SRC src/*.cpp)
include_directories(src/)

add_executable(${PROJECT_NAME} ${SRC})


# symlink the scripts folder to the build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts $<TARGET_FILE_DIR:${PROJECT_NAME}>/scripts
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scripts
    COMMENT "symbolic link resources folder from ${CMAKE_SOURCE_DIR}/scripts to $<TARGET_FILE_DIR:${PROJECT_NAME}>/scripts"
)