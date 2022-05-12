cmake_minimum_required(VERSION 3.22)
project(c_proc_monitor C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -pthread)
set(SOURCE_FILES main.c sts_queue/sts_queue.c sts_queue/sts_queue.h)

add_executable(c_proc_monitor ${SOURCE_FILES})
