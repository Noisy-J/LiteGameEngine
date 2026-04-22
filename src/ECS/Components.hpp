#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <unordered_map>
#include <string>
#include <any>

// Transform Component (всегда есть у сущности)
struct TransformComponent {
    sf::Vector2f Pos{ 0.f, 0.f };
    sf::Angle Rot{ sf::degrees(0.f) };
    sf::Vector2f Scale{ 1.f, 1.f };
};

// Sprite Component
struct SpriteComponent {
    std::shared_ptr<sf::Texture> texture;
    std::unique_ptr<sf::Sprite> sprite;  // Убрали std::optional
    int layer{ 0 };
    bool visible{ true };
};

// Velocity Component
struct VelocityComponent {
    sf::Vector2f Velocity{ 0.f, 0.f };
    float maxSpeed{ 1000.f };
    float damping{ 0.f };
};

// Tag Component (для идентификации)
struct TagComponent {
    std::string Tag;
};

// Script Component
struct ScriptComponent {
    std::string scriptPath;
    std::unordered_map<std::string, std::any> properties;
};

// Health Component
struct HealthComponent {
    int currentHealth{ 100 };
    int maxHealth{ 100 };
    bool isInvulnerable{ false };
    float invulnerabilityTime{ 0.f };
};

// Collider Component
struct ColliderComponent {
    sf::RectangleShape Collider;
    sf::Vector2f ColliderPosition{ 32.f, 32.f };
    sf::Angle ColliderRotation{ sf::degrees(0.f) };
    sf::Vector2f ColliderScale{ 1.f, 1.f };
    sf::Vector2f ColliderOrigin{ 0.f, 0.f };
    //float CollidRadius{ 16.f };
    sf::Vector2f ColliderSize{ 32.f,32.f };
};