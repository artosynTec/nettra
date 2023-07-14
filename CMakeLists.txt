cmake_minimum_required(VERSION 3.22.1)
project(nettra)

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(./src DIR_SRCS)

add_executable(nettra main.cpp ${DIR_SRCS})
