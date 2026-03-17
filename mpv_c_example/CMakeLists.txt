cmake_minimum_required(VERSION 3.10)

project(MPV_C_Example VERSION 1.0 LANGUAGES C)

add_executable(mpv_c_example main.c)

target_link_libraries(mpv_c_example PRIVATE mpv)
