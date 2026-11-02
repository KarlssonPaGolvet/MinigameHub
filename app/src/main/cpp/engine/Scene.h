#pragma once

#include "engine/Renderer.h"

class Engine;

// Shared colors so every screen looks consistent
namespace Theme {
constexpr Color Background = {0.529f, 0.0f, 0.0f, 1.0f};
constexpr Color Panel      = {0.30f, 0.02f, 0.02f, 1.0f};
constexpr Color Yellow     = {1.0f, 0.78f, 0.05f, 1.0f};
constexpr Color White      = {1.0f, 1.0f, 1.0f, 1.0f};
constexpr Color Dark       = {0.08f, 0.02f, 0.02f, 1.0f};
constexpr Color Green      = {0.20f, 0.65f, 0.25f, 1.0f};
constexpr Color Grey       = {0.55f, 0.45f, 0.45f, 1.0f};
}

// A screen in the app (menu, game list, a game...).
// The engine forwards touches and the back key to the active scene.
class Scene {
public:
    explicit Scene(Engine& engine) : engine(engine) {}
    virtual ~Scene() = default;

    virtual void draw(Renderer& r, float width, float height) = 0;

    // Called when a finger is lifted. (x, y) is where it was lifted.
    virtual void onTap(float x, float y) {}

    // Return true if the back press was handled; false lets the app close.
    virtual bool onBack() { return false; }

    // --- Called by the engine ---
    void touchDown(float x, float y) { touching = true; touchX = x; touchY = y; }
    void touchMove(float x, float y) { touchX = x; touchY = y; }
    void touchUp(float x, float y) {
        if (!touching) return;
        touching = false;
        onTap(x, y);
    }
    void touchCancel() { touching = false; }

protected:
    // True while a finger is held down inside `r` (used to highlight buttons)
    bool isPressed(const Rect& r) const { return touching && r.contains(touchX, touchY); }

    // Standard "< BACK" button position in the top-left corner
    static Rect backButtonRect(float w, float h) { return {w * 0.04f, h * 0.035f, w * 0.30f, h * 0.06f}; }

    Engine& engine;

private:
    bool touching = false;
    float touchX = 0, touchY = 0;
};
