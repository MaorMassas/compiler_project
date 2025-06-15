cmake_minimum_required(VERSION 3.28)
project(project)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(project
        main.c
        README.md
        util.c
        util.h
        pre_prossecor.h
        pre_prossecor.c
        first_pass.c
        first_pass.h
        first_pass.h
        table.h
        table.c
        second_pass.c
        second_pass.h)
