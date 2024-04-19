cmake_minimum_required(VERSION 3.27)
project(automates)

set(CMAKE_CXX_STANDARD 23)

add_executable(automates main.cpp
        StateMachine.hpp
        StateMachine.cpp)
