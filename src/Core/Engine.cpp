#include "Engine.hpp"
#include <iostream>
#include <cmath>

//#include "../ECS/Scene.hpp"
#include "ECS/Systems/MovementSystem.hpp"
#include "ECS/Systems/RenderSystem.hpp"
#include "Resources/ResourceManager.hpp"
#include "Design/StyleManager.hpp"

//Только инициализация
Engine::Engine() {
    initializeWindow();
    initializeImGui();
    initializeViewport();
    initializeSubsystems();
}

void Engine::initializeWindow() {
    m_Window.create(sf::VideoMode({ 1920, 1080 }), "Lite Game Engine v1.0.2b");
    m_Window.setFramerateLimit(60);
}

void Engine::initializeImGui() {
    if (!ImGui::SFML::Init(m_Window)) {
        throw std::runtime_error("Failed to initialize ImGui");
    }

    // Применяем Modern Dark стиль
    StyleManager::getInstance().applyStyle(StyleManager::Preset::ModernDark);
}

//Инициализация viewport с инверсией
void Engine::initializeViewport() {
    m_Viewport = std::make_unique<Viewport>(800, 600);

    sf::View view = m_Viewport->getView();
    auto size = m_Viewport->getSize();
    view.setCenter({ static_cast<float>(size.x) / 2.f, static_cast<float>(size.y) / 2.f });
    view.setSize({ static_cast<float>(size.x), -static_cast<float>(size.y) });
    m_Viewport->setView(view);
}

//Подключение подсистем
void Engine::initializeSubsystems() {
    m_InputManager = std::make_unique<InputManager>(m_Window, *m_Viewport);
    m_EditorUI = std::make_unique<EditorUI>(m_Window, m_Scene, *m_Viewport);
    m_TextureBrowser = std::make_unique<TextureBrowser>(m_Scene);
}

//Инициализация сущностей, ПОКА ТУТ, пользователь сам должен!
void Engine::initializeDefaultEntities() {
    // Загружаем текстуры
    /*m_DefaultTexture = ResourceManager::loadTexture("./assets/textures/hero.png");
    auto logoTexture = ResourceManager::loadTexture("./assets/textures/tusur_logo.png");

    // Создаём игрока
    m_Player = m_Scene.createEntity();
    m_Scene.setTexture(m_Player, m_DefaultTexture);
    m_Scene.setPosition(m_Player, { 400.f, 300.f });
    m_Scene.setRotation(m_Player, sf::degrees(0.f));
    m_Scene.setScale(m_Player, { 1.f, 1.f });
    m_Scene.setVelocity(m_Player, { 300.f, 0.f });

    // Второй игрок (уменьшенный)
    Entity player2 = m_Scene.createEntity();
    m_Scene.setTexture(player2, m_DefaultTexture);
    m_Scene.setPosition(player2, { 0.f, 0.f });
    m_Scene.setRotation(player2, sf::degrees(0.f));
    m_Scene.setScale(player2, { 0.5f, 0.5f });

    // Логотип
    Entity logo = m_Scene.createEntity();
    m_Scene.setTexture(logo, logoTexture);
    m_Scene.setPosition(logo, { 300.f, 100.f });
    m_Scene.setRotation(logo, sf::degrees(0.f));
    m_Scene.setScale(logo, { 1.f, 1.f });*/
}

void Engine::run() {
    initializeDefaultEntities();

    while (m_Window.isOpen()) {
        processFrame();
    }

    ImGui::SFML::Shutdown();
}

//Обработка времени
void Engine::processFrame() {
    sf::Time dt = m_Clock.restart();
    float deltaTime = dt.asSeconds();

    handleInput();
    update(deltaTime);
    render();
}

void Engine::handleInput() {
    m_InputManager->processInput(m_Scene, m_Player);
}

//Обновления в реал тайм
void Engine::update(float deltaTime) {
    // Обновление систем ECS
    MovementSystem::update(m_Scene, deltaTime);

    // Обновление ImGui
    ImGui::SFML::Update(m_Window, sf::seconds(deltaTime));

    // Обновление UI редактора
    Entity selectedEntity = m_InputManager->getSelectedEntity();
    m_EditorUI->update(selectedEntity, deltaTime);

    // Прямая передача состояния контекстного меню
    if (m_InputManager->isContextMenuOpen()) {
        sf::Vector2f menuPos = m_InputManager->getContextMenuPos();
        m_EditorUI->showContextMenu(menuPos);
        m_InputManager->closeContextMenu(); // Закрываем после передачи
    }
}

//Рендеринг
void Engine::render() {
    // Очистка
    m_Window.clear(sf::Color(40, 40, 40));
    m_Viewport->clear(sf::Color(100, 100, 100));

    // Рендеринг сцены во вьюпорт
    RenderSystem::drawSprites(m_Scene, m_Viewport->getRenderTexture());

    // Отладочная отрисовка
    Entity selectedEntity = m_InputManager->getSelectedEntity();
    m_Viewport->renderDebugOverlays(m_Scene, selectedEntity, m_InputManager->getMouseWorldPos());

    // Отображаем вьюпорт
    m_Viewport->display();

    // Рендеринг UI редактора
    m_EditorUI->render();

    // Финальный рендеринг
    ImGui::SFML::Render(m_Window);
    m_Window.display();
}