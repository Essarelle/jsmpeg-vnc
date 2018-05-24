cmake_minimum_required(VERSION 3.10)
project(jsmpeg_vnc C)

set(CMAKE_C_STANDARD 11)

if (NOT CMAKE_MODULE_PATH)
    set(CMAKE_MODULE_PATH /usr/share/apps/cmake/modules/)
endif ()

list(APPEND CMAKE_FIND_ROOT_PATH "${PROJECT_SOURCE_DIR}/ffmpeg")

include_directories(source/libwebsocket)

add_executable(jsmpeg_vnc
        source/app.c
        source/app.h
        source/encoder.c
        source/encoder.h
        source/grabber.c
        source/grabber.h
        source/jsmpeg-vnc.c
        source/server.c
        source/server.h
        source/timer.c
        source/timer.h
        )

find_package(FFmpeg COMPONENTS
        AVCODEC
        AVDEVICE
        AVFORMAT
        AVUTIL
        SWSCALE
        REQUIRED
        )

target_include_directories(jsmpeg_vnc PUBLIC ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(jsmpeg_vnc ${FFMPEG_LIBRARIES})

target_include_directories(jsmpeg_vnc PUBLIC "${PROJECT_SOURCE_DIR}/libwebsockets/include/")
target_link_libraries(jsmpeg_vnc "${PROJECT_SOURCE_DIR}/libwebsockets/lib/libwebsockets_static.a")

target_link_libraries(jsmpeg_vnc ws2_32)
