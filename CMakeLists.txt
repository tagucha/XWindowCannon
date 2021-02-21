cmake_minimum_required(VERSION 3.17)
project(untitled1 C)

set(CMAKE_C_STANDARD 99)

#add_compile_options(-I /opt/X11/lib -L /opt/X11/include -l X11 -Wall)
include_directories(/opt/X11/include)

add_executable(untitled1 main.c)
