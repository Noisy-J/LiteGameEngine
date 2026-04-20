#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "../ECS/Scene.hpp"
#include "../Rendering/Viewport.hpp"
#include "CameraController.hpp"
#include "EntityDragger.hpp"

class InputManager {
public:
    //Конструктор и деструктор
    InputManager(sf::RenderWindow& window, Viewport& viewport);
    ~InputManager() = default;

    //Главный обработчик нажатий
    void processInput(Scene& scene, Entity player);

    // Геттеры состояния
    Entity getSelectedEntity() const { return m_SelectedEntity; }
    sf::Vector2f getMouseWorldPos() const { return m_MouseWorldPos; }
    bool isContextMenuOpen() const { return m_ShowContextMenu; }
    sf::Vector2f getContextMenuPos() const { return m_ContextMenuPos; }

    // Управление контекстным меню
    void closeContextMenu() { m_ShowContextMenu = false; }

private:
    sf::RenderWindow& m_Window;
    Viewport& m_Viewport;

    //Ссылки на обекты и/или камеру
    std::unique_ptr<CameraController> m_CameraController;
    std::unique_ptr<EntityDragger> m_EntityDragger;

    //Обьект и камера
    Entity m_SelectedEntity{ INVALID_ENTITY };
    sf::Vector2f m_MouseWorldPos{ 0.f, 0.f };

    //Показ контекстного меню
    bool m_ShowContextMenu{ false };
    sf::Vector2f m_ContextMenuPos{ 0.f, 0.f };

    //События (клавиши)
    void handleEvents(Scene& scene);
    void handleMouseEvents(const sf::Event& event, Scene& scene);
    void handleKeyboardEvents(Scene& scene, Entity player);
    void updateMouseWorldPosition();
    void handleEntitySelection(const sf::Vector2i& pixelPos, Scene& scene);
};