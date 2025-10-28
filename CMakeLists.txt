cmake_minimum_required(VERSION 4.0)
project(DavinciGL C CXX)  # Add CXX to enable C++ support

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)  # Optional: set C++ standard

add_library(DavinciGL SHARED
        library.c
        rendering.cpp
        stb_image.h
)