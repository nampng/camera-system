cmake_minimum_required(VERSION 3.16)

project(gui LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(gstreamer-1.0 REQUIRED gstreamer-1.0)
pkg_check_modules(gstreamer-plugins-base-1.0 REQUIRED gstreamer-plugins-base-1.0)

add_executable(gui
        main.c
        thread.c
        thread.h
        stream.c
        stream.h
        fifo.c
        fifo.h
)

target_link_libraries(gui PRIVATE
        ${SDL2_LINK_LIBRARIES}
        ${gstreamer-1.0_LINK_LIBRARIES}
        ${gstreamer-plugins-base-1.0_LINK_LIBRARIES}
        gstapp-1.0
        pthread
)

target_include_directories(gui PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${gstreamer-1.0_INCLUDE_DIRS}
        ${gstreamer-plugins-base-1.0_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
)
