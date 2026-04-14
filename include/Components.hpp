#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>

struct TransformComponent {
    sf::Vector2f Pos{ 0.0f, 0.0f };
    sf::Vector2f Scale{ 1.0f, 1.0f };
    sf::Angle Rot = sf::degrees(0.0f);
};

struct SpriteComponent {
    // Используем optional, чтобы позволить "пустое" состояние компонента
    // и обойти отсутствие конструктора по умолчанию у sf::Sprite
    std::optional<sf::Sprite> sprite;

    // Конструктор по умолчанию теперь есть (optional будет пуст)
    SpriteComponent() = default;

    // Удобный конструктор для инициализации сразу с текстурой
    SpriteComponent(const sf::Texture& texture) : sprite(sf::Sprite(texture)) {}
};

struct VelocityComponent {
    sf::Vector2f Velocity{ 0.0f, 0.0f };
};

struct PhysicsComponent {
    float mass = 1.0f;
};