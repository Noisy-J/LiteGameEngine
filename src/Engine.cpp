#include "Engine.hpp"
#include <iostream>
#include <cmath>

#define GET_NAME(var) #var

// Объект (Движок)
Engine::Engine()
    : m_isDraggingCamera(false)
    , m_isDraggingEntity(false)
    , m_SelectedEntity(-1)
    , m_dragOffset({ 0.f, 0.f })
    , m_dragStartPos({ 0.f, 0.f })
    , m_dragThreshold(5.0f)
{
    m_Window.create(sf::VideoMode({ 1920, 1080 }), "Lite Game Engine v0.7b");
    m_Viewport.resize({ 800, 600 });
    m_Window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(m_Window)) {
        std::cerr << "Failed to initialize ImGui" << std::endl;
    }
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

        // Средняя кнопка мыши для движения камеры
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

        // Правая кнопка мыши - захват объекта
        if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseButton->button == sf::Mouse::Button::Right) {
                sf::Vector2i pixelPos = sf::Mouse::getPosition(m_Window);

                // Проверяем, что клик был внутри вьюпорта
                if (pixelPos.x >= m_viewportPos.x && pixelPos.x <= m_viewportPos.x + m_viewportSize.x &&
                    pixelPos.y >= m_viewportPos.y && pixelPos.y <= m_viewportPos.y + m_viewportSize.y) {

                    sf::Vector2f worldPos = m_Window.mapPixelToCoords(pixelPos, m_Viewport.getView());

                    m_SelectedEntity = -1;

                    // Ищем сущность под курсором (проверяем от задних к передним для правильного Z-order)
                    for (auto it = m_Scene.sprites.end(); it != m_Scene.sprites.begin(); ) {
                        --it;
                        auto& [entity, spriteComp] = *it;

                        // Получаем глобальные границы спрайта
                        sf::FloatRect bounds = spriteComp.sprite->getGlobalBounds();

                        // Проверяем попадание
                        if (bounds.contains(worldPos)) {
                            m_SelectedEntity = entity;
                            m_isDraggingEntity = true;

                            auto& tf = m_Scene.transforms[entity];

                            // ВАЖНО: Сохраняем смещение в МИРОВЫХ координатах
                            // При инвертированном Y, worldPos.y уже инвертирован
                            m_dragOffset.x = tf.Pos.x - worldPos.x;
                            m_dragOffset.y = tf.Pos.y - (-worldPos.y);

                            break;
                        }
                    }
                }
            }
        }

        // Отпускание правой кнопки мыши
        if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseButton->button == sf::Mouse::Button::Right) {
                m_isDraggingEntity = false;
            }
        }
    }

    // Перетаскивание объекта (обработка вне цикла событий для плавности)
    if (m_isDraggingEntity && m_SelectedEntity != -1) {
        sf::Vector2i pixelPos = sf::Mouse::getPosition(m_Window);

        // Проверяем, находится ли мышь внутри области вьюпорта
        if (pixelPos.x >= m_viewportPos.x && pixelPos.x <= m_viewportPos.x + m_viewportSize.x &&
            pixelPos.y >= m_viewportPos.y && pixelPos.y <= m_viewportPos.y + m_viewportSize.y) {

            sf::Vector2f worldPos = m_Window.mapPixelToCoords(pixelPos, m_Viewport.getView());

            // Обновляем позицию компонента - ПРЯМО ТАМ ГДЕ КУРСОР
            auto& tf = m_Scene.transforms[m_SelectedEntity];
            tf.Pos.x = worldPos.x + m_dragOffset.x;
            tf.Pos.y = -worldPos.y + m_dragOffset.y ;
        }
    }

    // Перетаскивание камеры
    if (m_isDraggingCamera) {
        sf::Vector2i currentMousePos = sf::Mouse::getPosition(m_Window);
        sf::Vector2i delta = currentMousePos - m_lastMousePos;

        if (delta.x != 0 || delta.y != 0) {
            sf::View view = m_Viewport.getView();
            // Инвертируем delta.y из-за инвертированного view
            view.move({ static_cast<float>(-delta.x), static_cast<float>(-delta.y) });
            m_Viewport.setView(view);
            m_lastMousePos = currentMousePos;
        }
    }

    // Обработка клавиатуры для движения игрока
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left)) {
        if (auto it = m_Scene.velocities.find(m_Player); it != m_Scene.velocities.end()) {
            it->second.Velocity.x = -300;
        }
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right)) {
        if (auto it = m_Scene.velocities.find(m_Player); it != m_Scene.velocities.end()) {
            it->second.Velocity.x = 300;
        }
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up)) {
        if (auto it = m_Scene.velocities.find(m_Player); it != m_Scene.velocities.end()) {
            it->second.Velocity.y = 300;  // Положительное значение для движения вверх (из-за инверсии)
        }
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down)) {
        if (auto it = m_Scene.velocities.find(m_Player); it != m_Scene.velocities.end()) {
            it->second.Velocity.y = -300;  // Отрицательное значение для движения вниз (из-за инверсии)
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
    ImGui::Text("Mouse Window: %d, %d", mousePos.x, mousePos.y);

    if (mousePos.x >= m_viewportPos.x && mousePos.x <= m_viewportPos.x + m_viewportSize.x &&
        mousePos.y >= m_viewportPos.y && mousePos.y <= m_viewportPos.y + m_viewportSize.y) {
        sf::Vector2f worldPos = m_Window.mapPixelToCoords(mousePos, m_Viewport.getView());
        ImGui::Text("Mouse World: %.1f, %.1f", worldPos.x, worldPos.y);
    }

    sf::View view = m_Viewport.getView();
    ImGui::Text("Camera Center: %.1f, %.1f", view.getCenter().x, view.getCenter().y);
    ImGui::Text("Camera Size: %.1f, %.1f", view.getSize().x, view.getSize().y);
    ImGui::Text("Is Dragging Camera: %s", m_isDraggingCamera ? "Yes" : "No");
    ImGui::Text("Is Dragging Entity: %s", m_isDraggingEntity ? "Yes" : "No");
    ImGui::Text("Selected Entity: %d", m_SelectedEntity);

    if (m_SelectedEntity != -1) {
        auto& tf = m_Scene.transforms[m_SelectedEntity];
        ImGui::Text("Entity Position: %.1f, %.1f", tf.Pos.x, tf.Pos.y);
        ImGui::Text("Drag Offset: %.1f, %.1f", m_dragOffset.x, m_dragOffset.y);
    }

    if (ImGui::Button("Reset Camera")) {
        float w = static_cast<float>(m_Viewport.getSize().x);
        float h = static_cast<float>(m_Viewport.getSize().y);
        view.setCenter({ w / 2.f, h / 2.f });
        view.setSize({ w, -h });
        m_Viewport.setView(view);
    }

    // Кнопка для сброса позиции выбранного объекта
    if (m_SelectedEntity != -1 && ImGui::Button("Reset Entity Position")) {
        auto& tf = m_Scene.transforms[m_SelectedEntity];
        tf.Pos = { 400.f, 300.f };
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
            if (ImGui::DragFloat2("Position", pos, 1.0f)) {
                tf.Pos = { pos[0], pos[1] };
            }

            float angle = tf.Rot.asDegrees();
            if (ImGui::DragFloat("Rotation", &angle, 1.0f)) {
                tf.Rot = sf::degrees(angle);
            }

            float scale[2] = { tf.Scale.x, tf.Scale.y };
            if (ImGui::DragFloat2("Scale", scale, 0.1f)) {
                tf.Scale = { scale[0], scale[1] };
            }
        }
    }

    // Инспектор Sprite
    if (auto it = scene->sprites.find(entity); it != scene->sprites.end()) {
        if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
            sf::Color color = it->second.sprite->getColor();
            float col[4] = {
                static_cast<float>(color.r) / 255.f,
                static_cast<float>(color.g) / 255.f,
                static_cast<float>(color.b) / 255.f,
                static_cast<float>(color.a) / 255.f
            };
            if (ImGui::ColorEdit4("Tint", col)) {
                it->second.sprite->setColor(sf::Color(
                    static_cast<std::uint8_t>(col[0] * 255),
                    static_cast<std::uint8_t>(col[1] * 255),
                    static_cast<std::uint8_t>(col[2] * 255),
                    static_cast<std::uint8_t>(col[3] * 255)
                ));
            }

            // Дополнительная информация о спрайте
            sf::FloatRect bounds = it->second.sprite->getGlobalBounds();
            ImGui::Text("Bounds: %.1f, %.1f, %.1f, %.1f",
                bounds.position.x, bounds.position.y,
                bounds.size.x, bounds.size.y);

            // Отладочная информация о текстуре
            const sf::Texture* tex = &it->second.sprite->getTexture();
            if (tex) {
                sf::Vector2u texSize = tex->getSize();
                ImGui::Text("Texture Size: %u x %u", texSize.x, texSize.y);
            }
        }
    }

    // Инспектор Velocity (если есть)
    if (auto it = scene->velocities.find(entity); it != scene->velocities.end()) {
        if (ImGui::CollapsingHeader("Velocity", ImGuiTreeNodeFlags_DefaultOpen)) {
            float vel[2] = { it->second.Velocity.x, it->second.Velocity.y };
            if (ImGui::DragFloat2("Velocity", vel, 10.0f)) {
                it->second.Velocity = { vel[0], vel[1] };
            }
        }
    }
}

// ОТРИСОВКА
void Engine::draw() {
    m_Window.clear(sf::Color(40, 40, 40));
    m_Viewport.clear(sf::Color(100, 100, 100));

    RenderSystem::drawSprites(m_Scene, m_Viewport);

    // Рисуем рамку вокруг выбранного объекта
    if (m_SelectedEntity != -1) {
        auto it = m_Scene.sprites.find(m_SelectedEntity);
        if (it != m_Scene.sprites.end()) {
            sf::FloatRect bounds = it->second.sprite->getGlobalBounds();
            sf::RectangleShape selectionRect({ bounds.size.x, bounds.size.y });
            selectionRect.setPosition({ bounds.position.x, bounds.position.y });
            selectionRect.setFillColor(sf::Color::Transparent);
            selectionRect.setOutlineColor(sf::Color::Yellow);
            selectionRect.setOutlineThickness(2.0f);
            m_Viewport.draw(selectionRect);
        }
    }

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