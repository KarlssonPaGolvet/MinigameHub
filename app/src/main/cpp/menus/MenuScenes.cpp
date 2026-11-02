#include "menus/MenuScenes.h"
#include "engine/Engine.h"
#include "games/hangman/HangmanScene.h"

#include <algorithm>
#include <cmath>
#include <memory>

// ---------------------------------------------------------------------------
// Main menu
// ---------------------------------------------------------------------------

void MainMenuScene::draw(Renderer& r, float w, float h) {
    // Title
    const std::string title = "MINIGAME HUB";
    float px = std::floor(w * 0.85f / (title.size() * 6.0f - 1.0f));
    r.textCentered(title, w * 0.5f + px * 0.5f, h * 0.22f + px * 0.5f, px, Theme::Dark); // shadow
    r.textCentered(title, w * 0.5f, h * 0.22f, px, Theme::Yellow);

    // GAMES button (image is 2:1)
    float bw = std::min(w * 0.75f, h * 0.6f);
    float bh = bw * 0.5f;
    gamesButton = {(w - bw) * 0.5f, h * 0.5f - bh * 0.5f, bw, bh};

    if (isPressed(gamesButton)) {
        // Shrink + darken slightly while held down
        Rect pressed = {gamesButton.x + bw * 0.03f, gamesButton.y + bh * 0.03f, bw * 0.94f, bh * 0.94f};
        r.image(engine.gamesButtonTex, pressed, {0.8f, 0.8f, 0.8f, 1.0f});
    } else {
        r.image(engine.gamesButtonTex, gamesButton);
    }

    float smallPx = std::max(2.0f, std::floor(px * 0.4f));
    r.textCentered("TAP TO PLAY", w * 0.5f, h * 0.5f + bh * 0.5f + h * 0.05f, smallPx, Theme::White);
}

void MainMenuScene::onTap(float x, float y) {
    if (gamesButton.contains(x, y)) {
        engine.setScene(std::make_unique<GameListScene>(engine));
    }
}

// ---------------------------------------------------------------------------
// Game list
// ---------------------------------------------------------------------------

void GameListScene::draw(Renderer& r, float w, float h) {
    backButton = backButtonRect(w, h);
    r.button(backButton, "< BACK", Theme::Panel, Theme::White, isPressed(backButton));

    const std::string title = "GAMES";
    float px = std::floor(w * 0.5f / (title.size() * 6.0f - 1.0f));
    r.textCentered(title, w * 0.5f, h * 0.17f, px, Theme::Yellow);

    hangmanButton = {w * 0.1f, h * 0.27f, w * 0.8f, h * 0.11f};
    r.button(hangmanButton, "HANGMAN", Theme::Yellow, Theme::Dark, isPressed(hangmanButton));

    Rect comingSoon = {w * 0.1f, h * 0.41f, w * 0.8f, h * 0.11f};
    r.button(comingSoon, "MORE SOON...", Theme::Panel, Theme::Grey, false);
}

void GameListScene::onTap(float x, float y) {
    if (backButton.contains(x, y)) {
        onBack();
    } else if (hangmanButton.contains(x, y)) {
        engine.setScene(std::make_unique<HangmanScene>(engine));
    }
}

bool GameListScene::onBack() {
    engine.setScene(std::make_unique<MainMenuScene>(engine));
    return true;
}
