#pragma once

#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "ArcadeHub", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "ArcadeHub", __VA_ARGS__)

class Engine {
public:
    Engine(android_app* app);
    ~Engine();

    bool initDisplay();
    void termDisplay();
    void drawFrame();

    android_app* app;

private:
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;

    // --- GRAPHICS VARIABLES ---
    GLuint shaderProgram;
    GLuint vbo;
    GLuint textureId;
    GLuint loadTexture(AAssetManager* assetManager, const char* fileName);
    
    // Helper function to compile shaders
    GLuint loadShader(GLenum type, const char* shaderSrc);
    void setupGraphics();
};