cmake_minimum_required(VERSION 3.8)
project(theamatriciana)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp Implementation.h Matrix.h)
add_executable(theamatriciana ${SOURCE_FILES})