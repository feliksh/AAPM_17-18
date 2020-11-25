cmake_minimum_required(VERSION 3.8)
project(MatrixLib)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        iterators.h
        matrix.h
        matrix_fwd.h
        matrix_wrap.h
        lazy_product.h
        main.cpp operations.h)

add_executable(MatrixLib ${SOURCE_FILES})