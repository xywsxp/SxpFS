cmake_minimum_required(VERSION 3.17)
project(sxpFS)

set(CMAKE_CXX_STANDARD 14)

add_executable(sxpFS main.cpp utils/inode.h utils/headers.h utils/inodeEngine.h sxpFS.h utils/disk.h)