#include "engine/Renderer.h"
#include "engine/Log.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "third_party/stb_image.h"

#include <algorithm>
#include <cctype>
#include <cmath>

// ---------------------------------------------------------------------------
// 5x7 pixel font. Each glyph is 7 rows, each row uses the low 5 bits
// (bit 4 = leftmost pixel).
// ---------------------------------------------------------------------------
namespace {

struct Glyph {
    char ch;
    uint8_t rows[7];
};

const Glyph kFont[] = {
    {'A', {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}},
    {'B', {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110}},
    {'C', {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110}},
    {'D', {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110}},
    {'E', {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111}},
    {'F', {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000}},
    {'G', {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111}},
    {'H', {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}},
    {'I', {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}},
    {'J', {0b00111, 0b00010, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100}},
    {'K', {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001}},
    {'L', {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111}},
    {'M', {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001}},
    {'N', {0b10001, 0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001}},
    {'O', {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},
    {'P', {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000}},
    {'Q', {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101}},
    {'R', {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001}},
    {'S', {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110}},
    {'T', {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}},
    {'U', {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},
    {'V', {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100}},
    {'W', {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010}},
    {'X', {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001}},
    {'Y', {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100}},
    {'Z', {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111}},
    {'0', {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110}},
    {'1', {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}},
    {'2', {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111}},
    {'3', {0b11111, 0b00010, 0b00100, 0b00010, 0b00001, 0b10001, 0b01110}},
    {'4', {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010}},
    {'5', {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110}},
    {'6', {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110}},
    {'7', {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000}},
    {'8', {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110}},
    {'9', {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100}},
    {'!', {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000, 0b00100}},
    {'?', {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b00000, 0b00100}},
    {'.', {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00100}},
    {',', {0b00000, 0b00000, 0b00000, 0b00000, 0b00100, 0b00100, 0b01000}},
    {':', {0b00000, 0b01100, 0b01100, 0b00000, 0b01100, 0b01100, 0b00000}},
    {'-', {0b00000, 0b00000, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000}},
    {'/', {0b00001, 0b00010, 0b00010, 0b00100, 0b01000, 0b01000, 0b10000}},
    {'<', {0b00010, 0b00100, 0b01000, 0b10000, 0b01000, 0b00100, 0b00010}},
    {'>', {0b01000, 0b00100, 0b00010, 0b00001, 0b00010, 0b00100, 0b01000}},
    {'\'', {0b00100, 0b00100, 0b01000, 0b00000, 0b00000, 0b00000, 0b00000}},
};

const uint8_t* findGlyph(char c) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    for (const Glyph& g : kFont) {
        if (g.ch == c) return g.rows;
    }
    return nullptr;
}

const char* kVertexShader =
    "#version 300 es\n"
    "layout(location = 0) in vec2 aPos;\n"
    "layout(location = 1) in vec2 aUV;\n"
    "layout(location = 2) in vec4 aColor;\n"
    "uniform vec2 uScreen;\n"
    "out vec2 vUV;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "  vUV = aUV;\n"
    "  vColor = aColor;\n"
    "  gl_Position = vec4(aPos.x / uScreen.x * 2.0 - 1.0,\n"
    "                     1.0 - aPos.y / uScreen.y * 2.0, 0.0, 1.0);\n"
    "}\n";

const char* kFragmentShader =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 vUV;\n"
    "in vec4 vColor;\n"
    "uniform sampler2D uTex;\n"
    "uniform int uUseTex;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "  vec4 c = vColor;\n"
    "  if (uUseTex == 1) c *= texture(uTex, vUV);\n"
    "  fragColor = c;\n"
    "}\n";

GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) return 0;
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        LOGE("Shader compile error:\n%s", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

constexpr int kFloatsPerVertex = 8; // x, y, u, v, r, g, b, a
constexpr float kPi = 3.14159265358979f;

} // namespace

// ---------------------------------------------------------------------------

bool Renderer::init() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, kVertexShader);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFragmentShader);
    if (!vs || !fs) return false;

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        LOGE("Program link error:\n%s", log);
        return false;
    }

    uScreenLoc = glGetUniformLocation(program, "uScreen");
    uTexLoc = glGetUniformLocation(program, "uTex");
    uUseTexLoc = glGetUniformLocation(program, "uUseTex");

    glGenBuffers(1, &vbo);
    verts.reserve(8192);
    return true;
}

void Renderer::shutdown() {
    // The GL context owns these objects and is destroyed right after this,
    // so just forget the handles.
    program = 0;
    vbo = 0;
    currentTex = 0;
    verts.clear();
}

void Renderer::begin(int screenW, int screenH) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(program);
    glUniform2f(uScreenLoc, static_cast<float>(screenW), static_cast<float>(screenH));
    glUniform1i(uTexLoc, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    const GLsizei stride = kFloatsPerVertex * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (const void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (const void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (const void*)(4 * sizeof(float)));

    currentTex = 0;
    verts.clear();
}

void Renderer::end() {
    flush();
}

void Renderer::flush() {
    if (verts.empty()) return;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STREAM_DRAW);

    glUniform1i(uUseTexLoc, currentTex != 0 ? 1 : 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTex);

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(verts.size() / kFloatsPerVertex));
    verts.clear();
}

void Renderer::useTexture(GLuint tex) {
    if (tex != currentTex) {
        flush();
        currentTex = tex;
    }
}

void Renderer::vertex(float x, float y, float u, float v, Color c) {
    verts.push_back(x);
    verts.push_back(y);
    verts.push_back(u);
    verts.push_back(v);
    verts.push_back(c.r);
    verts.push_back(c.g);
    verts.push_back(c.b);
    verts.push_back(c.a);
}

void Renderer::quad(float x0, float y0, float x1, float y1,
                    float x2, float y2, float x3, float y3, Color c) {
    vertex(x0, y0, 0, 0, c);
    vertex(x1, y1, 0, 0, c);
    vertex(x2, y2, 0, 0, c);
    vertex(x0, y0, 0, 0, c);
    vertex(x2, y2, 0, 0, c);
    vertex(x3, y3, 0, 0, c);
}

void Renderer::rect(const Rect& r, Color c) {
    useTexture(0);
    quad(r.x, r.y, r.x + r.w, r.y, r.x + r.w, r.y + r.h, r.x, r.y + r.h, c);
}

void Renderer::rectOutline(const Rect& r, float t, Color c) {
    rect({r.x, r.y, r.w, t}, c);
    rect({r.x, r.y + r.h - t, r.w, t}, c);
    rect({r.x, r.y + t, t, r.h - 2 * t}, c);
    rect({r.x + r.w - t, r.y + t, t, r.h - 2 * t}, c);
}

void Renderer::line(float x1, float y1, float x2, float y2, float t, Color c) {
    useTexture(0);
    float dx = x2 - x1, dy = y2 - y1;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.0001f) return;
    dx /= len;
    dy /= len;

    // Extend both ends by half the thickness so joints look solid
    float h = t * 0.5f;
    x1 -= dx * h; y1 -= dy * h;
    x2 += dx * h; y2 += dy * h;

    float nx = -dy * h, ny = dx * h;
    quad(x1 + nx, y1 + ny, x2 + nx, y2 + ny, x2 - nx, y2 - ny, x1 - nx, y1 - ny, c);
}

void Renderer::circle(float cx, float cy, float radius, float t, Color c, int segments) {
    useTexture(0);
    float rIn = radius - t * 0.5f, rOut = radius + t * 0.5f;
    for (int i = 0; i < segments; ++i) {
        float a0 = 2.0f * kPi * i / segments;
        float a1 = 2.0f * kPi * (i + 1) / segments;
        float c0 = std::cos(a0), s0 = std::sin(a0);
        float c1 = std::cos(a1), s1 = std::sin(a1);
        quad(cx + c0 * rIn, cy + s0 * rIn,
             cx + c0 * rOut, cy + s0 * rOut,
             cx + c1 * rOut, cy + s1 * rOut,
             cx + c1 * rIn, cy + s1 * rIn, c);
    }
}

void Renderer::image(GLuint texture, const Rect& r, Color tint) {
    if (texture == 0) return;
    useTexture(texture);
    vertex(r.x, r.y, 0, 0, tint);
    vertex(r.x + r.w, r.y, 1, 0, tint);
    vertex(r.x + r.w, r.y + r.h, 1, 1, tint);
    vertex(r.x, r.y, 0, 0, tint);
    vertex(r.x + r.w, r.y + r.h, 1, 1, tint);
    vertex(r.x, r.y + r.h, 0, 1, tint);
}

float Renderer::textWidth(const std::string& s, float px) {
    if (s.empty()) return 0;
    return (static_cast<float>(s.size()) * 6.0f - 1.0f) * px;
}

void Renderer::text(const std::string& s, float x, float y, float px, Color c) {
    float penX = x;
    for (char ch : s) {
        const uint8_t* rows = findGlyph(ch);
        if (rows) {
            for (int row = 0; row < 7; ++row) {
                for (int col = 0; col < 5; ++col) {
                    if (rows[row] & (1 << (4 - col))) {
                        rect({penX + col * px, y + row * px, px, px}, c);
                    }
                }
            }
        }
        penX += 6.0f * px;
    }
}

void Renderer::textCentered(const std::string& s, float cx, float cy, float px, Color c) {
    text(s, cx - textWidth(s, px) * 0.5f, cy - textHeight(px) * 0.5f, px, c);
}

void Renderer::button(const Rect& r, const std::string& label, Color bg, Color fg, bool pressed) {
    Color fill = bg;
    if (pressed) {
        fill.r *= 0.75f;
        fill.g *= 0.75f;
        fill.b *= 0.75f;
    }
    rect(r, fill);

    float border = std::max(2.0f, std::min(r.w, r.h) * 0.06f);
    rectOutline(r, border, {0.0f, 0.0f, 0.0f, 0.35f});

    float units = static_cast<float>(label.size()) * 6.0f - 1.0f;
    float px = std::min(r.h * 0.5f / 7.0f, r.w * 0.8f / units);
    px = std::floor(std::max(1.0f, px));
    textCentered(label, r.cx(), r.cy(), px, fg);
}

GLuint Renderer::loadTexture(AAssetManager* assetManager, const char* fileName) {
    AAsset* asset = AAssetManager_open(assetManager, fileName, AASSET_MODE_BUFFER);
    if (!asset) {
        LOGE("Failed to open asset: %s", fileName);
        return 0;
    }

    off_t length = AAsset_getLength(asset);
    std::vector<unsigned char> buffer(static_cast<size_t>(length));
    AAsset_read(asset, buffer.data(), static_cast<size_t>(length));
    AAsset_close(asset);

    int w = 0, h = 0, channels = 0;
    // Our renderer has Y pointing down, so the first image row is the top: no flip.
    stbi_set_flip_vertically_on_load(false);
    unsigned char* pixels = stbi_load_from_memory(buffer.data(), static_cast<int>(length),
                                                  &w, &h, &channels, 4);
    if (!pixels) {
        LOGE("Failed to decode image: %s", fileName);
        return 0;
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);

    // Restore whatever texture the batch is using
    glBindTexture(GL_TEXTURE_2D, currentTex);
    return texture;
}
