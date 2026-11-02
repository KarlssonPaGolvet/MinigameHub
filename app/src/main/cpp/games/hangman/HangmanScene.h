#pragma once

#include "engine/Scene.h"

#include <array>
#include <random>
#include <string>

class HangmanScene : public Scene {
public:
    explicit HangmanScene(Engine& engine);

    void draw(Renderer& r, float w, float h) override;
    void onTap(float x, float y) override;
    bool onBack() override;

private:
    static constexpr int kMaxWrong = 6;

    void newGame();
    void guess(char letter);
    bool isWon() const;
    bool isLost() const { return wrongGuesses >= kMaxWrong; }

    void drawGallows(Renderer& r, const Rect& area, float w);
    void drawWord(Renderer& r, float w, float h);
    void drawKeyboard(Renderer& r, float w, float h);

    std::mt19937 rng;
    std::string word;
    std::array<bool, 26> guessed{};
    int wrongGuesses = 0;

    // Layout from the last frame, used for hit-testing taps
    Rect backButton;
    Rect newGameButton;
    std::array<Rect, 26> keys{};
};
