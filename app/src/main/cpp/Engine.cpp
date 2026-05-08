#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Engine.h"

Engine::Engine(android_app* app) : app(app), display(EGL_NO_DISPLAY), surface(EGL_NO_SURFACE), context(EGL_NO_CONTEXT), width(0), height(0) {}

Engine::~Engine() {
    termDisplay();
}

bool Engine::initDisplay() {
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) return false;
    eglInitialize(display, nullptr, nullptr);

    const EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };
    
    EGLConfig config;
    EGLint numConfigs;
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    context = eglCreateContext(display, config, nullptr, contextAttribs);
    surface = eglCreateWindowSurface(display, config, app->window, nullptr);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGE("Unable to eglMakeCurrent");
        return false;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    glViewport(0, 0, width, height);
    glClearColor(0.529f, 0.0f, 0.0f, 1.0f); // Dark Red

    LOGI("EGL Initialized: %d x %d", width, height);
    setupGraphics();
    
    LOGI("EGL Initialized: %d x %d", width, height);
    return true;
}

GLuint Engine::loadShader(GLenum type, const char* shaderSrc) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) return 0;

    glShaderSource(shader, 1, &shaderSrc, nullptr);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* infoLog = new char[infoLen];
            glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
            LOGE("Error compiling shader:\n%s", infoLog);
            delete[] infoLog;
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void Engine::setupGraphics() {
    const char vShaderStr[] =
        "#version 300 es\n"
        "in vec4 vPosition;\n"
        "in vec2 vTexCoord;\n"
        "out vec2 TexCoord;\n"
        "void main() {\n"
        "  gl_Position = vPosition;\n"
        "  TexCoord = vTexCoord;\n"
        "}\n";

    const char fShaderStr[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D uTexture;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "  fragColor = texture(uTexture, TexCoord);\n" 
        "}\n";

    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vShaderStr);
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fShaderStr);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // X, Y, Z (Position) AND U, V (Texture Map)
    GLfloat vertices[] = {
        -0.6f,  0.2f, 0.0f,   0.0f, 1.0f, // Top Left
        -0.6f, -0.2f, 0.0f,   0.0f, 0.0f, // Bottom Left
         0.6f,  0.2f, 0.0f,   1.0f, 1.0f, // Top Right
         0.6f, -0.2f, 0.0f,   1.0f, 0.0f  // Bottom Right
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    textureId = loadTexture(app->activity->assetManager, "games_button.png"); 
}

void Engine::drawFrame() {
    if (display == EGL_NO_DISPLAY) return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable Alpha Blending for transparent PNG corners
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(shaderProgram);

    // Bind our button image
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Tell OpenGL where the X,Y,Z positions are (Starts at 0, skips 5 floats)
    GLuint positionLoc = glGetAttribLocation(shaderProgram, "vPosition");
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)0);

    // Tell OpenGL where the U,V coordinates are (Starts at 3, skips 5 floats)
    GLuint texCoordLoc = glGetAttribLocation(shaderProgram, "vTexCoord");
    glEnableVertexAttribArray(texCoordLoc);
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)(3 * sizeof(GLfloat)));

    // Draw the rectangle
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(positionLoc);
    glDisableVertexAttribArray(texCoordLoc);

    eglSwapBuffers(display, surface);
}

void Engine::termDisplay() {
    if (display != EGL_NO_DISPLAY) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context != EGL_NO_CONTEXT) eglDestroyContext(display, context);
        if (surface != EGL_NO_SURFACE) eglDestroySurface(display, surface);
        eglTerminate(display);
    }
    display = EGL_NO_DISPLAY;
    context = EGL_NO_CONTEXT;
    surface = EGL_NO_SURFACE;
}

GLuint Engine::loadTexture(AAssetManager* assetManager, const char* fileName) {
    AAsset* asset = AAssetManager_open(assetManager, fileName, AASSET_MODE_BUFFER);
    if (!asset) {
        LOGE("Failed to open asset: %s", fileName);
        return 0;
    }

    off_t length = AAsset_getLength(asset);
    unsigned char* buffer = new unsigned char[length];
    AAsset_read(asset, buffer, length);
    AAsset_close(asset);

    int width, height, channels;
    stbi_set_flip_vertically_on_load(true); // OpenGL reads bottom-to-top
    unsigned char* pixels = stbi_load_from_memory(buffer, length, &width, &height, &channels, 4);
    delete[] buffer;

    if (!pixels) {
        LOGE("Failed to decode image: %s", fileName);
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set texture scaling rules
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Upload to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Free the CPU memory
    stbi_image_free(pixels);
    
    return texture;
}