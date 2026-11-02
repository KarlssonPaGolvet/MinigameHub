#include <android_native_app_glue.h>
#include "engine/Engine.h"

// Window / lifecycle events
static void handle_cmd(struct android_app* app, int32_t cmd) {
    Engine* engine = (Engine*)app->userData;

    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != nullptr) {
                engine->initDisplay();
                engine->drawFrame(); // Draw immediately so it doesn't stay black
            }
            break;
        case APP_CMD_TERM_WINDOW:
            engine->termDisplay();
            break;
        default:
            break;
    }
}

// Touch and key events
static int32_t handle_input(struct android_app* app, AInputEvent* event) {
    Engine* engine = (Engine*)app->userData;
    return engine->handleInput(event);
}

void android_main(struct android_app* state) {
    Engine engine(state);
    state->userData = &engine;
    state->onAppCmd = handle_cmd;
    state->onInputEvent = handle_input;

    while (true) {
        int events;
        struct android_poll_source* source;

        // With a display: don't block (render every frame).
        // Without one (app in background): sleep until an event arrives.
        while (ALooper_pollOnce(engine.hasDisplay() ? 0 : -1, nullptr, &events, (void**)&source) >= 0) {
            if (source != nullptr) {
                source->process(state, source);
            }
            if (state->destroyRequested != 0) {
                engine.termDisplay();
                return;
            }
        }

        if (engine.hasDisplay()) {
            engine.drawFrame();
        }
    }
}
