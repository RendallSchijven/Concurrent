cmake_minimum_required(VERSION 3.9)
project(Concurrent)

set(CMAKE_CXX_STANDARD 11)

add_executable(Mandelbrot mandelbrot.cpp)
add_executable(Mutex dotproductMutex.cpp)
add_executable(Atomic dotproductAtomic.cpp)
add_executable(Tree tree.cpp)
add_executable(Threadpool threadpool.cpp)
add_executable(tttmc ttt.cpp ttt_mc.cpp)