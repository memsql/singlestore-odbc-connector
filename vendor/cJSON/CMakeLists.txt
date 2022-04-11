cmake_minimum_required(VERSION 3.0)

set(PROJECT_NAME cjson)
project(${PROJECT_NAME} LANGUAGES C)

add_library(${PROJECT_NAME} STATIC
    cJSON.c
)

target_include_directories(${PROJECT_NAME} INTERFACE .)
