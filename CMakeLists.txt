cmake_minimum_required(VERSION 2.8)
project(parakeet)

include(CheckCSourceCompiles)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -std=gnu89")

add_subdirectory(atomic)
add_subdirectory(shm)
add_subdirectory(mtx)

