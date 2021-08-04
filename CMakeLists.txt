cmake_minimum_required(VERSION 3.17)
project(RV32I-Assembler-Emulator)

set(CMAKE_CXX_STANDARD 14)

add_executable(RV32I-Assembler-Emulator src/main.cpp src/Assembler.cpp src/CPU.cpp src/Parser.cpp src/TestSystem.cpp src/Emulator.cpp include/Emulator.h)