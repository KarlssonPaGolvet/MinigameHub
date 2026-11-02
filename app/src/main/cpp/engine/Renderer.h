#pragma once

#include <GLES3/gl3.h>
#include <android/asset_manager.h>
#include <string>
#include <vector>

struct Color {
    float r, g, b, a;
};

struct Rect {
    float x = 0, y = 0, w = 0, h = 0;

    bool contains(float px, float py) const {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }
    float cx() const { return x + w * 0.5f; }
    float cy() const { return y + h * 0.5f; }
};

// Simple batched 2D renderer. Coordinates are in screen pixels with the
// origin in the top-left corner and Y pointing down.
class Renderer {
public:
    bool init();          // Call after a GL context has been made current
    void shutdown();      // Call when the GL context is about to be destroyed

    void begin(int screenW, int screenH);
    void end();

    void rect(const Rect& r, Color c);
    void rectOutline(const Rect& r, float thickness, Color c);
    void line(float x1, float y1, float x2, float y2, float thickness, Color c);
    void circle(float cx, float cy, float radius, float thickness, Color c, int segments = 40);
    void image(GLuint texture, const Rect& r, Color tint = {1, 1, 1, 1});

    // Text uses a built-in 5x7 pixel font. `px` is the size of one font pixel.
    void text(const std::string& s, float x, float y, float px, Color c);
    void textCentered(const std::string& s, float cx, float cy, float px, Color c);
    static float textWidth(const std::string& s, float px);
    static float textHeight(float px) { return 7.0f * px; }

    // A filled button with a border and a centered label that fits inside it.
    void button(const Rect& r, const std::string& label, Color bg, Color fg, bool pressed);

    GLuint loadTexture(AAssetManager* assetManager, const char* fileName);

private:
    void useTexture(GLuint tex);
    void flush();
    void vertex(float x, float y, float u, float v, Color c);
    void quad(float x0, float y0, float x1, float y1,
              float x2, float y2, float x3, float y3, Color c); // corners in order around the quad

    GLuint program = 0;
    GLuint vbo = 0;
    GLint uScreenLoc = -1;
    GLint uTexLoc = -1;
    GLint uUseTexLoc = -1;

    GLuint currentTex = 0;
    std::vector<float> verts;
};
