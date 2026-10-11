cmake_minimum_required(VERSION 3.10)
project(RISCV_Simulator)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(code main.c)

target_compile_options(code PRIVATE -Wall -Wextra -O2)