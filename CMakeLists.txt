include(FetchContent)

# Project settings
cmake_minimum_required(VERSION 3.14.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(FAST-SAN-LIB)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Normal chess lib

FetchContent_Declare(
    chess_lib
    GIT_REPOSITORY https://github.com/Disservin/chess-library
    GIT_TAG master
)

FetchContent_MakeAvailable(chess_lib)

add_library(ChessLib INTERFACE)
target_include_directories(ChessLib INTERFACE ${chess_lib_SOURCE_DIR}/include)

# New library target

add_library(FAST-SAN-LIB src/fast-san.cpp)
target_link_libraries(FAST-SAN-LIB PRIVATE ChessLib)


target_include_directories(FAST-SAN-LIB
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_subdirectory(benchmarks)
