cmake_minimum_required(VERSION 3.13)
project(sketch_apr07a)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(sketch_apr07a
        example_secrets.h
        README.md
        secrets.h
        sketch_apr07a.ino)
