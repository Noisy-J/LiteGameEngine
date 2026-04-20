#pragma once

#include "../Scene.hpp"
#include <SFML/Graphics.hpp>

class RenderSystem {
public:
    static void drawSprites(Scene& scene, sf::RenderTarget& target);
};