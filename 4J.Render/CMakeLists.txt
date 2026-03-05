cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)
project(4J_Render CXX)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
    4J_Render.cpp
    RendererCbuff.cpp
    RendererCore.cpp
    RendererMatrix.cpp
    RendererState.cpp
    RendererTexture.cpp
    RendererVertex.cpp
    stdafx.cpp
)
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
# giving a boost on the next macos implmentation 

if(UNIX AND NOT APPLE)
    find_package(PNG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
endif()

# Mimic cmake converter behaviour
target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h>"
)
