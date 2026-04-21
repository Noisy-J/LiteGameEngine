#include "InputManager.hpp"
#include "../Utils/CoordinateConverter.hpp"
#include <imgui-SFML.h> 
#include <iostream>

//Конструктор
InputManager::InputManager(sf::RenderWindow& window, Viewport& viewport)
    : m_Window(window)
    , m_Viewport(viewport) {
    m_CameraController = std::make_unique<CameraController>(m_Window, m_Viewport);
    m_EntityDragger = std::make_unique<EntityDragger>(m_Window, m_Viewport);
}

//Сканер нажатий
void InputManager::processInput(Scene& scene, Entity player) {
    updateMouseWorldPosition();
    handleEvents(scene);
    handleKeyboardEvents(scene, player);

    // Обновление перетаскивания
    if (m_EntityDragger->isDragging()) {
        m_EntityDragger->update(scene);
    }

    // Обновление камеры
    m_CameraController->update();
}

//Обновление позиции мыши
void InputManager::updateMouseWorldPosition() {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(m_Window);
    m_MouseWorldPos = CoordinateConverter::screenToWorld(pixelPos, m_Window, m_Viewport);
}

//
void InputManager::handleEvents(Scene& scene) {
    while (const std::optional<sf::Event> event = m_Window.pollEvent()) {
        // ImGui обработка
        ImGui::SFML::ProcessEvent(m_Window, *event);

        // Закрытие окна
        if (event->is<sf::Event::Closed>()) {
            m_Window.close();
        }

        // События камеры
        m_CameraController->handleEvent(*event);

        // События мыши
        handleMouseEvents(*event, scene);
    }
}

void InputManager::handleMouseEvents(const sf::Event& event, Scene& scene) {
    // Правая кнопка - выделение и контекстное меню
    if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseBtn->button == sf::Mouse::Button::Right) {
            sf::Vector2i pixelPos = { mouseBtn->position.x, mouseBtn->position.y };
            handleEntitySelection(pixelPos, scene);
        }

        // Клик в другом месте закрывает контекстное меню
        if (mouseBtn->button != sf::Mouse::Button::Right && m_ShowContextMenu) {
            m_ShowContextMenu = false;
        }
    }

    // Отпускание правой кнопки
    if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseBtn->button == sf::Mouse::Button::Right) {
            m_EntityDragger->release();
        }
    }
}

void InputManager::handleEntitySelection(const sf::Vector2i& pixelPos, Scene& scene) {
    if (!m_Viewport.isPointInside(pixelPos)) return;

    m_SelectedEntity = m_EntityDragger->tryGrabEntity(scene, m_MouseWorldPos);

    //Если нет обьекта в поле клика, предложить создать
    if (m_SelectedEntity == INVALID_ENTITY) {
        m_ShowContextMenu = true;
        m_ContextMenuPos = { static_cast<float>(pixelPos.x), static_cast<float>(pixelPos.y) };
    }
    else {
        m_ShowContextMenu = false;
    }
}

void InputManager::handleKeyboardEvents(Scene& scene, Entity player) {
    // Удаление сущности
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Delete)) {
        if (m_SelectedEntity != INVALID_ENTITY && m_SelectedEntity != player) {
            scene.destroyEntity(m_SelectedEntity);
            m_SelectedEntity = INVALID_ENTITY;
            m_EntityDragger->release();
        }
    }

    // Управление игроком
    auto velIt = scene.velocities.find(player);
    if (velIt == scene.velocities.end()) return;

    //Скорость
    auto& velocity = velIt->second.Velocity;
    velocity.x = 0.f;
    velocity.y = 0.f;

    //ЗАМЕНИТЬ, ПОЛУЧАТЬ У ПЕРСОНАЖА
    const float speed = 300.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
        velocity.x = -speed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
        velocity.x = speed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) {
        velocity.y = -speed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
        velocity.y = speed;
    }

    // Ctrl+S - Сохранить
   /*if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl) &&
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
        // Вызываем сохранение через callback
        if (m_SaveCallback) {
            m_SaveCallback();
        }
    }

    // Ctrl+O - Открыть
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl) &&
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::O)) {
        if (m_LoadCallback) {
            m_LoadCallback();
        }
    }*/
}