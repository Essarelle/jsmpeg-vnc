cmake_minimum_required(VERSION 3.10)
project(jsmpeg_vnc C)

set(CMAKE_C_STANDARD 11)

if (NOT CMAKE_MODULE_PATH)
    set(CMAKE_MODULE_PATH /usr/share/apps/cmake/modules/)
endif ()

add_executable(${PROJECT_NAME}
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

list(APPEND CMAKE_FIND_ROOT_PATH "${PROJECT_SOURCE_DIR}/ffmpeg")
find_package(FFmpeg COMPONENTS
        AVCODEC
        AVDEVICE
        AVFORMAT
        AVUTIL
        SWSCALE
        REQUIRED
        )
target_include_directories(${PROJECT_NAME} PUBLIC ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBRARIES})

set(LWS_WITHOUT_TESTAPPS ON CACHE BOOL "")
set(LWS_WITH_SSL OFF CACHE BOOL "")
add_subdirectory(libwebsockets)
get_directory_property(HDR_PUBLIC DIRECTORY libwebsockets DEFINITION HDR_PUBLIC)
foreach (INCLUDE_FILE ${HDR_PUBLIC})
    get_filename_component(INCLUDE_FILE_PARENT_DIR "${INCLUDE_FILE}" DIRECTORY)
    target_include_directories(${PROJECT_NAME} PUBLIC "${INCLUDE_FILE_PARENT_DIR}")
endforeach ()
target_link_libraries(${PROJECT_NAME} websockets)
