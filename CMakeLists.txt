cmake_minimum_required(VERSION 3.9)
project(Concurrent)

set(CMAKE_CXX_STANDARD 11)

add_executable(Mandelbrot mandelbrot.cpp)
add_executable(Mutex dotproductMutex.cpp)
add_executable(Atomic dotproductAtomic.cpp)