#pragma once
#include <SFML/Graphics.hpp>
#include "../Rendering/Viewport.hpp"

class CoordinateConverter {
public:
    static sf::Vector2f screenToWorld(
        const sf::Vector2i& screenPos,
        const sf::RenderWindow& window,
        Viewport& viewport);

    static sf::Vector2i worldToScreen(
        const sf::Vector2f& worldPos,
        const sf::RenderWindow& window,
        Viewport& viewport);
};