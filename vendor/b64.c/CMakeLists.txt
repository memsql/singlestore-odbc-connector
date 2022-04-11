cmake_minimum_required(VERSION 3.0)

set(PROJECT_NAME b64c)
project(${PROJECT_NAME} LANGUAGES C)

add_library(${PROJECT_NAME} STATIC
    buffer.c
    decode.c
    encode.c
)

target_include_directories(${PROJECT_NAME} INTERFACE .)
