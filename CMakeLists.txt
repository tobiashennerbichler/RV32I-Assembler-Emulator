cmake_minimum_required(VERSION 3.16)
project(RV32I-Assembler-Emulator)

set(CMAKE_CXX_STANDARD 14)

add_executable(rv32i_assemble_emulate src/main.cpp src/Assembler.cpp src/CPU.cpp src/Parser.cpp src/TestSystem.cpp src/Emulator.cpp include/Emulator.h)