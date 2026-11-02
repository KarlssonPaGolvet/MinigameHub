#pragma once

#include "engine/Scene.h"

// The first screen: app title + the big GAMES button
class MainMenuScene : public Scene {
public:
    using Scene::Scene;
    void draw(Renderer& r, float w, float h) override;
    void onTap(float x, float y) override;

private:
    Rect gamesButton;
};

// List of available games
class GameListScene : public Scene {
public:
    using Scene::Scene;
    void draw(Renderer& r, float w, float h) override;
    void onTap(float x, float y) override;
    bool onBack() override;

private:
    Rect backButton;
    Rect hangmanButton;
};
