cmake_minimum_required(VERSION 4.0)
project(DavinciGL C)

set(CMAKE_C_STANDARD 11)

add_library(DavinciGL SHARED library.c
        library.h)
