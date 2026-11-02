#pragma once

#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <memory>

#include "engine/Log.h"
#include "engine/Renderer.h"
#include "engine/Scene.h"

class Engine {
public:
    explicit Engine(android_app* app);
    ~Engine();

    bool initDisplay();
    void termDisplay();
    void drawFrame();
    int32_t handleInput(AInputEvent* event);
    bool hasDisplay() const { return display != EGL_NO_DISPLAY; }

    // Switch to another screen. Takes effect after the current event/frame,
    // so it is safe to call from inside a scene's own methods.
    void setScene(std::unique_ptr<Scene> next) { pendingScene = std::move(next); }

    android_app* app;
    Renderer renderer;

    // Textures (reloaded every time the display is created)
    GLuint gamesButtonTex = 0;

private:
    void applyPendingScene();

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;

    std::unique_ptr<Scene> scene;
    std::unique_ptr<Scene> pendingScene;
};
