cmake_minimum_required(VERSION 3.8)
project(MatrixLib)

set(CMAKE_CXX_STANDARD 14)

SET(CMAKE_CXX_FLAGS -pthread)

set(SOURCE_FILES
        example5.cc
        iterators.h
        matrix.h
        matrix_fwd.h
        matrix_wrap.h
        operations.h exceptions.h)

add_executable(MatrixLib ${SOURCE_FILES})