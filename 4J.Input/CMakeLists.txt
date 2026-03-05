cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)
project(4J_Input CXX)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
    4J_Input.cpp
    INP_ForceFeedback.cpp
    INP_Keyboard.cpp
    INP_Main.cpp
    INP_StringCheck.cpp
    stdafx.cpp
    ../Minecraft.Client/Platform/Linux/Stubs/LinuxStubs.h
)
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Mimic cmake converter behaviour
target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h>"
)
