cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)
project(4J_Storage CXX)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
    4J_Storage.cpp
    STO_DLC.cpp
    STO_Main.cpp
    STO_SaveGame.cpp
    stdafx.cpp
)
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Mimic cmake converter behaviour
target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h>"
)
