#include "games/hangman/HangmanScene.h"
#include "engine/Engine.h"
#include "menus/MenuScenes.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <memory>

namespace {

const char* const kWords[] = {
    "APPLE", "BANANA", "GUITAR", "ROCKET", "PLANET", "GARDEN", "PUZZLE", "CASTLE",
    "DRAGON", "WIZARD", "PIRATE", "JUNGLE", "WINTER", "SUMMER", "ORANGE", "PURPLE",
    "MONKEY", "TURTLE", "PENGUIN", "GIRAFFE", "ELEPHANT", "DOLPHIN", "OCTOPUS", "KANGAROO",
    "COMPUTER", "KEYBOARD", "TELEPHONE", "BICYCLE", "AIRPLANE", "SUBMARINE", "VOLCANO", "ISLAND",
    "PYRAMID", "TREASURE", "LANTERN", "BLANKET", "PILLOW", "CANDLE", "WINDOW", "LADDER",
    "PANCAKE", "SANDWICH", "CHOCOLATE", "COOKIE", "POPCORN", "PRETZEL", "AVOCADO", "PEPPER",
    "THUNDER", "RAINBOW", "TORNADO", "BLIZZARD", "SUNSHINE", "MEADOW", "FOREST", "CANYON",
    "KNIGHT", "PRINCESS", "ROBOT", "ZOMBIE", "VAMPIRE", "GHOST", "MUMMY", "ALIEN",
    "SOCCER", "HOCKEY", "TENNIS", "BOXING", "SKIING", "JOYSTICK", "ARCADE", "MINIGAME",
};

constexpr Color kKeyCorrect = Theme::Green;
constexpr Color kKeyWrong   = {0.35f, 0.08f, 0.08f, 1.0f};
constexpr Color kWood       = {0.78f, 0.52f, 0.26f, 1.0f};
constexpr Color kRed        = {1.0f, 0.35f, 0.30f, 1.0f};

} // namespace

HangmanScene::HangmanScene(Engine& engine) : Scene(engine), rng(std::random_device{}()) {
    newGame();
}

void HangmanScene::newGame() {
    std::uniform_int_distribution<size_t> pick(0, std::size(kWords) - 1);
    std::string next = kWords[pick(rng)];
    // Avoid the same word twice in a row
    while (next == word) next = kWords[pick(rng)];
    word = next;
    guessed.fill(false);
    wrongGuesses = 0;
}

void HangmanScene::guess(char letter) {
    int idx = letter - 'A';
    if (idx < 0 || idx >= 26 || guessed[idx]) return;
    guessed[idx] = true;
    if (word.find(letter) == std::string::npos) {
        ++wrongGuesses;
    }
}

bool HangmanScene::isWon() const {
    for (char c : word) {
        if (!guessed[c - 'A']) return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------------------

void HangmanScene::draw(Renderer& r, float w, float h) {
    backButton = backButtonRect(w, h);
    r.button(backButton, "< BACK", Theme::Panel, Theme::White, isPressed(backButton));

    // Lives counter in the top-right
    std::string lives = "MISSES " + std::to_string(wrongGuesses) + "/" + std::to_string(kMaxWrong);
    float livesPx = std::floor(std::max(2.0f, w * 0.0075f));
    r.text(lives, w * 0.96f - Renderer::textWidth(lives, livesPx),
           backButton.cy() - Renderer::textHeight(livesPx) * 0.5f, livesPx,
           wrongGuesses >= kMaxWrong - 1 ? kRed : Theme::White);

    drawGallows(r, {w * 0.2f, h * 0.13f, w * 0.6f, h * 0.30f}, w);
    drawWord(r, w, h);

    // Status line
    float statusPx = std::floor(std::max(2.0f, w * 0.012f));
    if (isWon()) {
        r.textCentered("YOU WIN!", w * 0.5f, h * 0.59f, statusPx, Theme::Green);
    } else if (isLost()) {
        r.textCentered("YOU LOSE!", w * 0.5f, h * 0.59f, statusPx, kRed);
    } else {
        r.textCentered("PICK A LETTER", w * 0.5f, h * 0.59f, statusPx * 0.75f, Theme::White);
    }

    if (isWon() || isLost()) {
        newGameButton = {w * 0.2f, h * 0.68f, w * 0.6f, h * 0.09f};
        r.button(newGameButton, "NEW GAME", Theme::Yellow, Theme::Dark, isPressed(newGameButton));
    } else {
        newGameButton = {};
        drawKeyboard(r, w, h);
    }
}

void HangmanScene::drawGallows(Renderer& r, const Rect& g, float w) {
    float t = std::max(3.0f, w * 0.012f);

    float bottom = g.y + g.h;
    float poleX = g.x + g.w * 0.15f;
    float ropeX = g.x + g.w * 0.65f;

    // The gallows itself is always visible
    r.line(g.x, bottom, g.x + g.w * 0.6f, bottom, t, kWood);   // base
    r.line(poleX, bottom, poleX, g.y, t, kWood);               // pole
    r.line(poleX, g.y, ropeX, g.y, t, kWood);                  // beam
    r.line(poleX, g.y + g.h * 0.18f, poleX + g.w * 0.15f, g.y, t * 0.8f, kWood); // brace
    r.line(ropeX, g.y, ropeX, g.y + g.h * 0.15f, t * 0.6f, Theme::Yellow);       // rope

    // The person appears one part per wrong guess
    Color body = isLost() ? kRed : Theme::White;
    float headR = g.h * 0.1f;
    float headCY = g.y + g.h * 0.15f + headR;
    float neckY = headCY + headR;
    float hipY = neckY + g.h * 0.30f;
    float armY = neckY + g.h * 0.07f;
    float limbX = g.w * 0.12f;

    if (wrongGuesses >= 1) r.circle(ropeX, headCY, headR, t, body);
    if (wrongGuesses >= 2) r.line(ropeX, neckY, ropeX, hipY, t, body);
    if (wrongGuesses >= 3) r.line(ropeX, armY, ropeX - limbX, armY + g.h * 0.11f, t, body);
    if (wrongGuesses >= 4) r.line(ropeX, armY, ropeX + limbX, armY + g.h * 0.11f, t, body);
    if (wrongGuesses >= 5) r.line(ropeX, hipY, ropeX - limbX, hipY + g.h * 0.22f, t, body);
    if (wrongGuesses >= 6) r.line(ropeX, hipY, ropeX + limbX, hipY + g.h * 0.22f, t, body);
}

void HangmanScene::drawWord(Renderer& r, float w, float h) {
    float n = static_cast<float>(word.size());
    float slot = std::min(w * 0.9f / n, w * 0.11f);
    float startX = (w - slot * n) * 0.5f;
    float baseY = h * 0.52f;
    float px = std::floor(std::max(2.0f, slot * 0.55f / 5.0f));
    float lineT = std::max(2.0f, px * 0.6f);

    bool lost = isLost();
    for (size_t i = 0; i < word.size(); ++i) {
        float cx = startX + slot * (i + 0.5f);
        r.rect({cx - slot * 0.38f, baseY, slot * 0.76f, lineT}, Theme::White);

        char c = word[i];
        bool shown = guessed[c - 'A'];
        if (shown || lost) {
            // Missed letters are revealed in red when the game is lost
            Color col = shown ? Theme::Yellow : kRed;
            float ty = baseY - Renderer::textHeight(px) - px * 1.5f;
            r.text(std::string(1, c), cx - Renderer::textWidth("A", px) * 0.5f, ty, px, col);
        }
    }
}

void HangmanScene::drawKeyboard(Renderer& r, float w, float h) {
    const int cols = 7;
    float top = h * 0.65f;
    float gap = w * 0.015f;
    float keyW = (w * 0.94f - gap * (cols - 1)) / cols;
    float keyH = std::min(keyW * 1.2f, (h * 0.96f - top - gap * 3) / 4.0f);

    for (int i = 0; i < 26; ++i) {
        int row = i / cols;
        int col = i % cols;
        int inRow = std::min(cols, 26 - row * cols);           // last row has 5 keys
        float rowW = inRow * keyW + (inRow - 1) * gap;
        float x = (w - rowW) * 0.5f + col * (keyW + gap);
        float y = top + row * (keyH + gap);
        keys[i] = {x, y, keyW, keyH};

        char letter = static_cast<char>('A' + i);
        std::string label(1, letter);
        if (!guessed[i]) {
            r.button(keys[i], label, Theme::Yellow, Theme::Dark, isPressed(keys[i]));
        } else if (word.find(letter) != std::string::npos) {
            r.button(keys[i], label, kKeyCorrect, Theme::White, false);
        } else {
            r.button(keys[i], label, kKeyWrong, Theme::Grey, false);
        }
    }
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

void HangmanScene::onTap(float x, float y) {
    if (backButton.contains(x, y)) {
        onBack();
        return;
    }

    if (isWon() || isLost()) {
        if (newGameButton.contains(x, y)) newGame();
        return;
    }

    for (int i = 0; i < 26; ++i) {
        if (keys[i].contains(x, y)) {
            guess(static_cast<char>('A' + i));
            return;
        }
    }
}

bool HangmanScene::onBack() {
    engine.setScene(std::make_unique<GameListScene>(engine));
    return true;
}
