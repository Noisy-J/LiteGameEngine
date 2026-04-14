#include "Engine.hpp"
#include <iostream>

#define GET_NAME(var) #var

// Объект (Движок)
Engine::Engine() {
    m_Window.create(sf::VideoMode({ 1920, 1080 }), "Lite Game Engine v0.7");
    m_Viewport.resize({ 800, 600 });
    m_Window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(m_Window)) {
        std::cerr << "Failed to initialize ImGui" << std::endl;
    }

    m_isDraggingCamera = false;
}

// При запуске
void Engine::run() {

    // Устанавливаем начальный view с инверсией
    sf::View view = m_Viewport.getView();
    float w = static_cast<float>(m_Viewport.getSize().x);
    float h = static_cast<float>(m_Viewport.getSize().y);
    view.setCenter({ w / 2.f, h / 2.f });
    view.setSize({ w, -h });  // С инверсией
    m_Viewport.setView(view);

    // Новая сущность игрок
    m_Player = m_Scene.createEntity();

    m_Texture = ResourceManager::loadTexture("./assets/hero.png");
    sf::Texture logo_png = ResourceManager::loadTexture("./assets/tusur_logo.png");

    m_Scene.setTexture(m_Player, m_Texture);
    m_Scene.setPosition(m_Player, { 400, 300 });
    m_Scene.setRotation(m_Player, 0);
    m_Scene.setScale(m_Player, { 1, 1 });
    m_Scene.setVelocity(m_Player, { 300, 0 });

    Entity player2 = m_Scene.createEntity();
    m_Scene.setTexture(player2, m_Texture);
    m_Scene.setPosition(player2, { 0 , 0 });
    m_Scene.setRotation(player2, 0);
    m_Scene.setScale(player2, { 0.5, 0.5 });

    Entity logo = m_Scene.createEntity();
    m_Scene.setTexture(logo, logo_png);
    m_Scene.setPosition(logo, { 300, 100 });

    // Пока окно открыто
    while (m_Window.isOpen()) {
        sf::Time dtTime = m_Clock.restart();

        input();
        update(dtTime, &m_Scene);
        draw();
    }
}

// Читаем кнопки (Эвенты)
void Engine::input() {
    while (const std::optional<sf::Event> event = m_Window.pollEvent()) {
        ImGui::SFML::ProcessEvent(m_Window, *event);

        if (event->is<sf::Event::Closed>()) {
            m_Window.close();
        }

        // Правая кнопка мыши для движения камеры
        if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseButton->button == sf::Mouse::Button::Middle) { 
                m_isDraggingCamera = true;
                m_lastMousePos = sf::Mouse::getPosition(m_Window);
                m_Window.setMouseCursorVisible(false);
            }
        }

        if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseButton->button == sf::Mouse::Button::Middle) {
                m_isDraggingCamera = false;
                m_Window.setMouseCursorVisible(true);
            }
        }

        // Колесико мыши для зума
        if (const auto* mouseWheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
            sf::View view = m_Viewport.getView();
            float zoomFactor = 1.0f - (mouseWheel->delta * 0.1f);
            view.zoom(zoomFactor);
            m_Viewport.setView(view);
        }

        if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseButton->button == sf::Mouse::Button::Right) {
                sf::Vector2i pixelPos = sf::Mouse::getPosition(m_Window);

                // Проверяем, что клик был внутри вьюпорта
                if (pixelPos.x >= m_viewportPos.x && pixelPos.x <= m_viewportPos.x + m_viewportSize.x &&
                    pixelPos.y >= m_viewportPos.y && pixelPos.y <= m_viewportPos.y + m_viewportSize.y) {

                    sf::Vector2f worldPos = m_Window.mapPixelToCoords(pixelPos, m_Viewport.getView());

                    m_SelectedEntity = -1;

                    for (auto& [entity, spriteComp] : m_Scene.sprites) {
                        if (spriteComp.sprite->getGlobalBounds().contains(worldPos)) {
                            m_SelectedEntity = entity;
                            m_isDraggingEntity = true;
                            auto& tf = m_Scene.transforms[entity];
                            m_dragOffset.x = tf.Pos.x - worldPos.x;
                            m_dragOffset.y = tf.Pos.y - (-worldPos.y);
                            break;
                        }
                    }
                }
            }
        }

        if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseButton->button == sf::Mouse::Button::Right) {
                // 1. Переводим экранные координаты мыши в мировые координаты вьюпорта
                sf::Vector2i pixelPos = sf::Mouse::getPosition(m_Window);
                sf::Vector2f worldPos = m_Window.mapPixelToCoords(pixelPos, m_Viewport.getView());

                m_SelectedEntity = -1; // Сбрасываем выбор

                // 2. Picker: Перебираем все спрайты и проверяем попадание
                for (auto& [entity, spriteComp] : m_Scene.sprites) {
                    if (spriteComp.sprite->getGlobalBounds().contains(worldPos)) {
                        m_SelectedEntity = entity;

                        // 3. Подготовка к перетаскиванию (Guizmos)
                        m_isDraggingEntity = true;
                        auto& tf = m_Scene.transforms[entity];
                        m_dragOffset.x = tf.Pos.x - worldPos.x;
                        m_dragOffset.y = tf.Pos.y - (-worldPos.y);
                        break;
                    }
                }
            }
        }

        if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseButton->button == sf::Mouse::Button::Right) {
                m_isDraggingEntity = false;
            }
        }
    }


    if (m_isDraggingEntity && m_SelectedEntity != -1) {
        // Получаем позицию мыши относительно окна
        sf::Vector2i pixelPos = sf::Mouse::getPosition(m_Window);

        // Проверяем, находится ли мышь внутри области вьюпорта
        if (pixelPos.x >= m_viewportPos.x && pixelPos.x <= m_viewportPos.x + m_viewportSize.x &&
            pixelPos.y >= m_viewportPos.y && pixelPos.y <= m_viewportPos.y + m_viewportSize.y) {

            // Конвертируем координаты мыши из оконных в координаты вьюпорта
            sf::Vector2f worldPos = m_Window.mapPixelToCoords(pixelPos, m_Viewport.getView());

            // Обновляем позицию компонента
            auto& tf = m_Scene.transforms[m_SelectedEntity];
            tf.Pos.x = worldPos.x + m_dragOffset.x;
            tf.Pos.y = (-worldPos.y) + m_dragOffset.y;
        }
    }


    // Перетаскивание камеры
    if (m_isDraggingCamera) {
        sf::Vector2i currentMousePos = sf::Mouse::getPosition(m_Window);
        sf::Vector2i delta = currentMousePos - m_lastMousePos;  // Обратите внимание: current - last

        if (delta.x != 0 || delta.y != 0) {
            sf::View view = m_Viewport.getView();
            view.move({ static_cast<float>(-delta.x), static_cast<float>(-delta.y) });  // Минусы для интуитивного движения
            m_Viewport.setView(view);
            m_lastMousePos = currentMousePos;
        }
    }


    // Обработка клавиатуры для движения игрока
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left)) {
        if (auto it = m_Scene.velocities.find(m_Player); it != m_Scene.velocities.end()) {
            it->second.Velocity.x = -100;
        }
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right)) {
        if (auto it = m_Scene.velocities.find(m_Player); it != m_Scene.velocities.end()) {
            it->second.Velocity.x = 100;
        }
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up)) {
        if (auto it = m_Scene.velocities.find(m_Player); it != m_Scene.velocities.end()) {
            it->second.Velocity.y = -100;
        }
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down)) {
        if (auto it = m_Scene.velocities.find(m_Player); it != m_Scene.velocities.end()) {
            it->second.Velocity.y = 100;
        }
    }
    else {
        if (auto it = m_Scene.velocities.find(m_Player); it != m_Scene.velocities.end()) {
            it->second.Velocity.x = 0;
            it->second.Velocity.y = 0;
        }
    }
}

// ЛОГИКА
void Engine::update(sf::Time dt, Scene* scene) {
    ImGui::SFML::Update(m_Window, dt);
    MovementSystem::update(*scene, dt.asSeconds());

    // Вызов в Engine::update:
    ImGui::Begin("Inspector");
    renderInspector(m_SelectedEntity, &m_Scene);
    ImGui::End();

    ImGui::Begin("Content Browser");
    ImGui::SetWindowSize({ static_cast<float>(m_Window.getSize().x), 0 }, ImGuiCond_Always);
    ImGui::SetWindowSize({ static_cast<float>(m_Window.getSize().x),  static_cast<float>(ImGui::GetWindowSize().y) });
    ImGui::SetWindowPos({ 0,  ((static_cast<float>(m_Window.getSize().y) - (static_cast<float>(ImGui::GetWindowSize().y)))) });
    ImGui::End();

    // Отладочная информация
    ImGui::Begin("Debug");
    ImGui::SetWindowPos({ 0,  0 });
    sf::Vector2i mousePos = sf::Mouse::getPosition(m_Window);
    ImGui::Text("Mouse Position: %d, %d", mousePos.x, mousePos.y);

    sf::View view = m_Viewport.getView();
    ImGui::Text("Camera Center: %.1f, %.1f", view.getCenter().x, view.getCenter().y);
    ImGui::Text("Camera Size: %.1f, %.1f", view.getSize().x, view.getSize().y);
    ImGui::Text("Is Dragging: %s", m_isDraggingCamera ? "Yes" : "No");

    if (ImGui::Button("Reset Camera")) {
        float w = static_cast<float>(0);
        float h = static_cast<float>(0);
        view.setCenter({ w / 2.f, h / 2.f });
        m_Viewport.setView(view);
    }
    ImGui::End();
}

void Engine::renderInspector(Entity entity, Scene* scene) {
    if (entity == -1) {
        ImGui::Text("No entity selected");
        return;
    }

    ImGui::Text("Selected Entity ID: %d", entity);
    ImGui::Separator();

    // Инспектор Transform
    if (auto it = scene->transforms.find(entity); it != scene->transforms.end()) {
        auto& tf = it->second;
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            float pos[2] = { tf.Pos.x, tf.Pos.y };
            if (ImGui::DragFloat2("Position", pos, 1.0f)) tf.Pos = { pos[0], pos[1] };

            float angle = tf.Rot.asDegrees();
            if (ImGui::DragFloat("Rotation", &angle, 1.0f)) tf.Rot = sf::degrees(angle);

            float scale[2] = { tf.Scale.x, tf.Scale.y };
            if (ImGui::DragFloat2("Scale", scale, 0.1f)) tf.Scale = { scale[0], scale[1] };
        }
    }

    // Инспектор Sprite (Цвет) (ДОДЕЛАТЬ!!!)
    if (auto it = scene->sprites.find(entity); it != scene->sprites.end()) {
        if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
            sf::Color color = it->second.sprite->getColor();
            float col[4] = { color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f };
            if (ImGui::ColorEdit4("Tint", col)) {
                it->second.sprite->setColor(sf::Color(col[0] * 255, col[1] * 255, col[2] * 255, col[3] * 255));
            }
        }
    }
}


// ОТРИСОВКА
void Engine::draw() {
    m_Window.clear(sf::Color(40, 40, 40));
    m_Viewport.clear(sf::Color(100, 100, 100));

    RenderSystem::drawSprites(m_Scene, m_Viewport);
    m_Viewport.display();

    ImGui::Begin("Viewport");

    // Сохраняем позицию вьюпорта в окне
    ImVec2 viewportScreenPos = ImGui::GetCursorScreenPos();
    m_viewportPos = { viewportScreenPos.x, viewportScreenPos.y };

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    m_viewportSize = { viewportSize.x, viewportSize.y };

    if (viewportSize.x > 0 && viewportSize.y > 0) {
        if (static_cast<unsigned int>(viewportSize.x) != m_Viewport.getSize().x ||
            static_cast<unsigned int>(viewportSize.y) != m_Viewport.getSize().y) {

            // Сохраняем текущий зум
            sf::View oldView = m_Viewport.getView();
            sf::Vector2f oldSize = oldView.getSize();

            // Вычисляем коэффициент зума
            float zoomX = oldSize.x / static_cast<float>(m_Viewport.getSize().x);
            float zoomY = oldSize.y / static_cast<float>(m_Viewport.getSize().y);

            m_Viewport.resize({ static_cast<unsigned int>(viewportSize.x),
                               static_cast<unsigned int>(viewportSize.y) });

            // Применяем тот же зум к новому размеру
            sf::View newView = m_Viewport.getView();
            newView.setSize({ static_cast<float>(viewportSize.x) * zoomX,
                             -static_cast<float>(viewportSize.y) * std::abs(zoomY) });
            m_Viewport.setView(newView);
        }

        sf::Vector2f imageSize(viewportSize.x, viewportSize.y);
        ImGui::Image(m_Viewport.getTexture(), imageSize);


    }
    ImGui::End();

    ImGui::SFML::Render(m_Window);
    m_Window.display();
}