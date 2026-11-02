cmake_minimum_required(VERSION 3.22.1)
project("arcadehub")

add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)
target_include_directories(native_app_glue PUBLIC
    ${ANDROID_NDK}/sources/android/native_app_glue
)

add_library(main SHARED
    main.cpp

    # Core: window, input, drawing, scene switching
    engine/Engine.cpp
    engine/Renderer.cpp

    # Menu screens
    menus/MenuScenes.cpp

    # Games (one folder per game)
    games/hangman/HangmanScene.cpp
)

# Includes are written relative to this folder, e.g. #include "engine/Engine.h"
target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_options(main PRIVATE -u ANativeActivity_onCreate)

target_link_libraries(main
    native_app_glue
    android
    log
    EGL
    GLESv3
)
