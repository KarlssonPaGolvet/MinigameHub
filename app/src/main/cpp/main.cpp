#include <android_native_app_glue.h>
#include "Engine.h"

// The event handler callback
static void handle_cmd(struct android_app* app, int32_t cmd) {
    // Extract our custom Engine object from the user data pointer
    Engine* engine = (Engine*)app->userData;

    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (engine->app->window != nullptr) {
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

void android_main(struct android_app* state) {
    // Instantiate the engine and attach it to the Android state
    Engine engine(state);
    state->userData = &engine;
    state->onAppCmd = handle_cmd;

    int events;
    struct android_poll_source* source;

    // Game loop
    while (true) {
        while (ALooper_pollAll(0, nullptr, &events, (void**)&source) >= 0) {
            if (source != nullptr) {
                source->process(state, source);
            }
            if (state->destroyRequested != 0) {
                engine.termDisplay();
                return;
            }
        }

        // Only draw if we have an active display
        if (engine.app->window != nullptr) {
            engine.drawFrame();
        }
    }
}