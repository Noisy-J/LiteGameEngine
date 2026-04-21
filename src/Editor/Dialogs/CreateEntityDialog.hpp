#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <functional>
#include "ECS/Scene.hpp"
#include "ECS/Components.hpp"
#include <iostream>

class CreateEntityDialog {
public:
    CreateEntityDialog(Scene& scene);
    ~CreateEntityDialog() = default;

    void open(const sf::Vector2f& worldPosition);
    void render();
    bool isOpen() const { return m_IsOpen; }

    // Callback при создании сущности
    void setOnEntityCreated(std::function<void(Entity)> callback) { m_OnEntityCreated = callback; }

private:
    Scene& m_Scene;
    bool m_IsOpen{ false };
    sf::Vector2f m_WorldPosition{ 0.f, 0.f };

    // Настройки сущности
    char m_EntityName[64] = "New Entity";

    // Компоненты для добавления
    bool m_AddSprite{ false };
    bool m_AddVelocity{ false };
    bool m_AddCollider{ false };
    bool m_AddHealth{ false };
    bool m_AddTag{ false };

    // Настройки компонентов
    // Sprite
    std::string m_SelectedTexturePath;
    sf::Color m_SpriteColor{ sf::Color::White };
    int m_SpriteLayer{ 0 };

    // Velocity
    sf::Vector2f m_InitialVelocity{ 0.f, 0.f };
    float m_MaxSpeed{ 1000.f };
    float m_Damping{ 0.f };

    // Collider
    ColliderComponent::Shape m_ColliderShape{ ColliderComponent::Shape::Box };
    sf::Vector2f m_BoxSize{ 32.f, 32.f };
    float m_CircleRadius{ 16.f };
    sf::Vector2f m_ColliderOffset{ 0.f, 0.f };
    bool m_IsTrigger{ false };
    bool m_IsStatic{ false };

    // Health
    int m_MaxHealth{ 100 };
    int m_CurrentHealth{ 100 };

    // Tag
    char m_TagName[32] = "Untagged";

    // UI состояние
    int m_CurrentStep{ 0 }; // 0: Basic, 1: Components, 2: Component Settings
    std::vector<std::string> m_AvailableTextures;
    int m_SelectedTextureIndex{ -1 };

    // Callback
    std::function<void(Entity)> m_OnEntityCreated;

    // Методы
    void renderBasicInfo();
    void renderComponentSelection();
    void renderComponentSettings();
    void renderSpriteSettings();
    void renderVelocitySettings();
    void renderColliderSettings();
    void renderHealthSettings();
    void renderTagSettings();

    void createEntity();
    void scanTextures();
    //void loadTexturePreview();

    // Вспомогательные методы
    const char* getStepName(int step) const;
    bool canProceed() const;
};