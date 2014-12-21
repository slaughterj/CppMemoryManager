cmake_minimum_required(VERSION 2.8)
project(memmanager)

set(CMAKE_BUILD_TYPE Release)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Source)

add_library(memmanager SHARED ${CMAKE_CURRENT_SOURCE_DIR}/Source/MemoryManager.cpp)

