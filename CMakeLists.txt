cmake_minimum_required(VERSION 3.5)
project(shell C)

set(CMAKE_C_STANDARD 11)

add_executable(
        shell
        src/main.c
        src/cd.c
        src/cd.h
        src/exit.c
        src/exit.h
        src/command.c
        src/command.h
        src/execute.c
        src/execute.h
        src/recordatorio.h
        src/recordatorio.c
        src/prompt.c
        src/prompt.h
        src/favs.c
        src/favs.h
)
