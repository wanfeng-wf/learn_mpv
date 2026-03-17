cmake_minimum_required(VERSION 3.10)

project(MPV_IPC_Example VERSION 1.0 LANGUAGES C)

add_executable(play play.c mpv_ipc.c)
target_link_libraries(play PRIVATE cjson)

add_executable(time time.c mpv_ipc.c)
target_link_libraries(time PRIVATE cjson)

add_executable(pause pause.c mpv_ipc.c)
target_link_libraries(pause PRIVATE cjson)

add_executable(quit quit.c mpv_ipc.c)
target_link_libraries(quit PRIVATE cjson)
