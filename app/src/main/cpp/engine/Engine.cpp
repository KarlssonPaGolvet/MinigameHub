#include "engine/Engine.h"
#include "menus/MenuScenes.h"

#include <android/input.h>

Engine::Engine(android_app* app)
    : app(app), display(EGL_NO_DISPLAY), surface(EGL_NO_SURFACE), context(EGL_NO_CONTEXT),
      width(0), height(0) {
    scene = std::make_unique<MainMenuScene>(*this);
}

Engine::~Engine() {
    termDisplay();
}

bool Engine::initDisplay() {
    termDisplay(); // in case an old one is still around

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) return false;
    eglInitialize(display, nullptr, nullptr);

    const EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_NONE
    };

    EGLConfig config;
    EGLint numConfigs = 0;
    if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs) || numConfigs < 1) {
        LOGE("No suitable EGL config");
        termDisplay();
        return false;
    }

    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    context = eglCreateContext(display, config, nullptr, contextAttribs);
    surface = eglCreateWindowSurface(display, config, app->window, nullptr);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGE("Unable to eglMakeCurrent");
        termDisplay();
        return false;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);
    LOGI("EGL Initialized: %d x %d", width, height);

    if (!renderer.init()) {
        LOGE("Renderer init failed");
    }
    gamesButtonTex = renderer.loadTexture(app->activity->assetManager, "games_button.png");
    return true;
}

void Engine::termDisplay() {
    if (display != EGL_NO_DISPLAY) {
        renderer.shutdown();
        gamesButtonTex = 0;
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context != EGL_NO_CONTEXT) eglDestroyContext(display, context);
        if (surface != EGL_NO_SURFACE) eglDestroySurface(display, surface);
        eglTerminate(display);
    }
    display = EGL_NO_DISPLAY;
    context = EGL_NO_CONTEXT;
    surface = EGL_NO_SURFACE;
}

void Engine::applyPendingScene() {
    if (pendingScene) {
        scene = std::move(pendingScene);
    }
}

void Engine::drawFrame() {
    if (display == EGL_NO_DISPLAY) return;
    applyPendingScene();

    // Re-query every frame so rotation/resizes are handled
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);
    glViewport(0, 0, width, height);

    glClearColor(Theme::Background.r, Theme::Background.g, Theme::Background.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    renderer.begin(width, height);
    scene->draw(renderer, static_cast<float>(width), static_cast<float>(height));
    renderer.end();

    eglSwapBuffers(display, surface);
}

int32_t Engine::handleInput(AInputEvent* event) {
    int32_t type = AInputEvent_getType(event);

    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        switch (action) {
            case AMOTION_EVENT_ACTION_DOWN:   scene->touchDown(x, y); break;
            case AMOTION_EVENT_ACTION_MOVE:   scene->touchMove(x, y); break;
            case AMOTION_EVENT_ACTION_UP:     scene->touchUp(x, y); break;
            case AMOTION_EVENT_ACTION_CANCEL: scene->touchCancel(); break;
            default: break;
        }
        applyPendingScene();
        return 1;
    }

    if (type == AINPUT_EVENT_TYPE_KEY && AKeyEvent_getKeyCode(event) == AKEYCODE_BACK) {
        if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_UP) {
            if (!scene->onBack()) {
                ANativeActivity_finish(app->activity); // back on the main menu closes the app
            }
            applyPendingScene();
        }
        return 1;
    }

    return 0;
}
