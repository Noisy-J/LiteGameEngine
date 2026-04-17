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
    , m_zoomX(1)
    , m_zoomY(1)
{
    m_Window.create(sf::VideoMode({ 1920, 1080 }), "Lite Game Engine v0.9");
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

                    sf::Vector2f worldPos = m_Window.mapPixelToCoords(pixelPos, m_Window.getView());

                    //worldPos = { worldPos.x - (m_viewportPos.x),
                      //  worldPos.y - (m_viewportPos.y) };

                    worldPos = screenToWorld(pixelPos);

                    m_SelectedEntity = -1;

                    // Ищем сущность под курсором
                    for (auto it = m_Scene.sprites.end(); it != m_Scene.sprites.begin(); ) {
                        --it;
                        auto& [entity, spriteComp] = *it;

                        sf::FloatRect bounds = spriteComp.sprite->getGlobalBounds();

                        if (bounds.contains(worldPos)) {
                            m_SelectedEntity = entity;
                            m_isDraggingEntity = true;

                            auto& tf = m_Scene.transforms[entity];
                    
                            // Просто сохраняем разницу между позицией объекта и позицией мыши
                            m_dragOffset.x = tf.Pos.x - worldPos.x;
                            m_dragOffset.y = tf.Pos.y - worldPos.y;

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

            sf::Vector2f worldPos = m_Window.mapPixelToCoords(pixelPos, m_Window.getView());

            //worldPos = { worldPos.x - (m_viewportPos.x),
              //  worldPos.y - (m_viewportPos.y) };

            worldPos = screenToWorld(pixelPos);

            // Обновляем позицию компонента - ПРЯМО ТАМ ГДЕ КУРСОР
            auto& tf = m_Scene.transforms[m_SelectedEntity];
            tf.Pos.x = worldPos.x + m_dragOffset.x;
            tf.Pos.y = worldPos.y + m_dragOffset.y ;
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
            it->second.Velocity.y = -300;  // Положительное значение для движения вверх
        }
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down)) {
        if (auto it = m_Scene.velocities.find(m_Player); it != m_Scene.velocities.end()) {
            it->second.Velocity.y = 300;  // Отрицательное значение для движения вниз
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
    sf::View view = m_Viewport.getView();

    // Вызов в Engine::update:
    ImGui::Begin("Inspector");
    renderInspector(m_SelectedEntity, &m_Scene);
    ImGui::End();

    ImGui::Begin("Content Browser");
    ImGui::SetWindowSize({ static_cast<float>(m_Window.getSize().x), 0 }, ImGuiCond_Always);
    ImGui::SetWindowSize({ static_cast<float>(m_Window.getSize().x),  static_cast<float>(ImGui::GetWindowSize().y) });
    ImGui::SetWindowPos({ 0,  ((static_cast<float>(m_Window.getSize().y) - (static_cast<float>(ImGui::GetWindowSize().y)))) });
    ImGui::End();

// Получаем текущую позицию мыши
    ImGui::Begin("Debug Panel");

    sf::Vector2i mousePos = sf::Mouse::getPosition(m_Window);
    sf::Vector2f worldPos = m_Window.mapPixelToCoords(mousePos, m_Window.getView());

    //worldPos = { worldPos.x - (m_viewportPos.x),
        //worldPos.y - (m_viewportPos.y) };

    worldPos = screenToWorld(mousePos);

    ImGui::Text("Mouse Window"); ImGui::NextColumn();
    ImGui::Text("%d, %d", mousePos.x, mousePos.y); ImGui::NextColumn();

    ImGui::Text("Mouse in Viewport"); ImGui::NextColumn();
    ImGui::Text("%.2f, %.2f", worldPos.x, worldPos.y); ImGui::NextColumn();

    // Проверяем каждый объект на попадание мыши
    ImGui::Separator();
    ImGui::Text("OBJECTS UNDER MOUSE:");
    ImGui::Separator();

    for (auto& [entity, spriteComp] : m_Scene.sprites) {
        sf::FloatRect bounds = spriteComp.sprite->getGlobalBounds();
        bool contains = bounds.contains(worldPos);

        if (contains) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Entity %d", entity);
            ImGui::SameLine();
            ImGui::Text("- Bounds: (%.1f,%.1f) %.1fx%.1f",
                bounds.position.x, bounds.position.y,
                bounds.size.x, bounds.size.y);
        }
    }

    if (m_SelectedEntity != -1) {
        auto spriteIt = m_Scene.sprites.find(m_SelectedEntity);
        if (spriteIt != m_Scene.sprites.end()) {
            auto& tf = m_Scene.transforms[m_SelectedEntity];
            sf::FloatRect bounds = spriteIt->second.sprite->getGlobalBounds();

            ImGui::Separator();
            ImGui::Text("SELECTED ENTITY %d:", m_SelectedEntity);
            ImGui::Separator();

            ImGui::Text("Transform Pos"); ImGui::NextColumn();
            ImGui::Text("%.2f, %.2f", tf.Pos.x, tf.Pos.y); ImGui::NextColumn();

            ImGui::Text("Bounds"); ImGui::NextColumn();
            ImGui::Text("(%.2f,%.2f) %.2fx%.2f",
                bounds.position.x, bounds.position.y,
                bounds.size.x, bounds.size.y); ImGui::NextColumn();

            ImGui::Text("Bounds Center"); ImGui::NextColumn();
            sf::Vector2f center = bounds.getCenter();
            ImGui::Text("%.2f, %.2f", center.x, center.y); ImGui::NextColumn();

            // Разница между позицией мыши и центром bounds
            ImGui::Text("Mouse to Center offset"); ImGui::NextColumn();
            ImGui::Text("%.2f, %.2f", worldPos.x - center.x, worldPos.y - center.y); ImGui::NextColumn();

            // Проверка contains
            bool contains = bounds.contains(worldPos);
            ImGui::Text("Mouse in Bounds"); ImGui::NextColumn();
            ImGui::TextColored(contains ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1),
                "%s", contains ? "YES" : "NO"); ImGui::NextColumn();
        }
    }
    ImGui::End();

    //if (m_isDraggingEntity == false && m_SelectedEntity == -1) {
        //ImGui::Begin("Panel");
        //ImGui::Button("Create Entity");

        //ImGui::SetWindowPos({ static_cast<float>(worldPos.x), static_cast<float>(worldPos.y) });
        //ImGui::End();
    //}

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


    m_Viewport.display();

    ImGui::Begin("Viewport");

    ImVec2 viewportSizeWindow = ImGui::GetWindowSize();
    m_viewportSizeWindow = { viewportSizeWindow.x, viewportSizeWindow.y };

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    m_viewportSize = { viewportSize.x, viewportSize.y };

    // Сохраняем позицию вьюпорта в окне
    //ImVec2 viewportScreenPos = ImGui::GetCursorScreenPos();
    ImVec2 viewportScreenPos = ImGui::GetCursorScreenPos();
    m_viewportPos = { viewportScreenPos.x, viewportScreenPos.y };

    // Рисуем зону клика, преобразуя координаты мыши в "неинвертированное" пространство
    for (auto& [entity, spriteComp] : m_Scene.sprites) {
        // 1. Обычный хитбокс спрайта (зеленый) - это РЕАЛЬНАЯ зона клика
        sf::FloatRect bounds = spriteComp.sprite->getGlobalBounds();
        sf::RectangleShape hitbox;
        hitbox.setPosition({ bounds.position.x, bounds.position.y });
        hitbox.setSize({ bounds.size.x, bounds.size.y });
        hitbox.setFillColor(sf::Color(0, 255, 0, 30));
        hitbox.setOutlineColor(sf::Color::Green);
        hitbox.setOutlineThickness(1.0f);
        m_Viewport.draw(hitbox);

        // 2. Получаем позицию мыши и преобразуем её ТАК ЖЕ как при клике
        sf::Vector2i pixelPos = sf::Mouse::getPosition(m_Window);
        sf::Vector2f worldPos = m_Window.mapPixelToCoords(pixelPos, m_Window.getView());

        //worldPos = { worldPos.x - (m_viewportPos.x),
        //worldPos.y - (m_viewportPos.y) };

        worldPos = screenToWorld(pixelPos);

        // 3. Проверяем, находится ли мышь в bounds (без всякой инверсии)
        bool isMouseOver = bounds.contains(worldPos);

        // 4. Рисуем дополнительную подсветку если мышь над объектом
        if (isMouseOver) {
            sf::RectangleShape hoverHighlight;
            hoverHighlight.setPosition({ bounds.position.x, bounds.position.y });
            hoverHighlight.setSize({ bounds.size.x, bounds.size.y });
            hoverHighlight.setFillColor(sf::Color(255, 255, 0, 40));
            hoverHighlight.setOutlineColor(sf::Color::Yellow);
            hoverHighlight.setOutlineThickness(2.0f);
            m_Viewport.draw(hoverHighlight);
        }

        // 5. Рисуем точку, где находится мышь в мировых координатах
        sf::CircleShape mousePoint(3);
        mousePoint.setPosition(worldPos);
        mousePoint.setOrigin({ 3, 3 });
        mousePoint.setFillColor(sf::Color::Magenta);
        m_Viewport.draw(mousePoint);

        // 6. Для выбранного объекта - дополнительная информация
        if (entity == m_SelectedEntity) {
            // Жирная белая рамка
            sf::RectangleShape debugRect;
            debugRect.setPosition({ bounds.position.x, bounds.position.y });
            debugRect.setSize({ bounds.size.x, bounds.size.y });
            debugRect.setFillColor(sf::Color::Transparent);
            debugRect.setOutlineColor(sf::Color::White);
            debugRect.setOutlineThickness(3.0f);
            m_Viewport.draw(debugRect);

            // Крестик в центре bounds
            sf::Vector2f center = bounds.getCenter();
            sf::VertexArray cross(sf::PrimitiveType::Lines, 4);
            cross[0].position = { center.x - 15, center.y };
            cross[1].position = { center.x + 15, center.y };
            cross[2].position = { center.x, center.y - 15 };
            cross[3].position = { center.x, center.y + 15 };
            cross[0].color = cross[1].color = cross[2].color = cross[3].color = sf::Color::White;
            m_Viewport.draw(cross);

            // Рисуем позицию tf.Pos (синяя точка)
            auto& tf = m_Scene.transforms[entity];
            sf::CircleShape tfPosPoint(5);
            tfPosPoint.setPosition(tf.Pos);
            tfPosPoint.setOrigin({ 5, 5 });
            tfPosPoint.setFillColor(sf::Color::Blue);
            m_Viewport.draw(tfPosPoint);
        }
    }

    // Отдельно рисуем позицию мыши для отладки
    sf::Vector2i pixelPos = sf::Mouse::getPosition(m_Window);
    sf::Vector2f worldPos = m_Window.mapPixelToCoords(pixelPos, m_Window.getView());
    
    //worldPos = { worldPos.x - (m_viewportPos.x),
    //worldPos.y - (m_viewportPos.y) };
    worldPos = screenToWorld(pixelPos);


    // Рисуем координатную сетку в точке мыши
    sf::VertexArray mouseCross(sf::PrimitiveType::Lines, 4);
    mouseCross[0].position = { worldPos.x - 20, worldPos.y };
    mouseCross[1].position = { worldPos.x + 20, worldPos.y };
    mouseCross[2].position = { worldPos.x, worldPos.y - 20 };
    mouseCross[3].position = { worldPos.x, worldPos.y + 20 };
    mouseCross[0].color = mouseCross[1].color = mouseCross[2].color = mouseCross[3].color = sf::Color(255, 0, 255, 200);
    m_Viewport.draw(mouseCross);

    if (viewportSize.x > 0 && viewportSize.y > 0) {
        if (static_cast<unsigned int>(viewportSize.x) != m_Viewport.getSize().x ||
            static_cast<unsigned int>(viewportSize.y) != m_Viewport.getSize().y) {

            // Сохраняем текущий зум
            sf::View oldView = m_Viewport.getView();
            sf::Vector2f oldSize = oldView.getSize();

            // Вычисляем коэффициент зума
            float zoomX = oldSize.x / static_cast<float>(m_Viewport.getSize().x);
            float zoomY = oldSize.y / static_cast<float>(m_Viewport.getSize().y);

            m_zoomX = zoomX;
            m_zoomY = zoomY;

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

// Функция для конвертации экранных координат в мировые координаты вьюпорта
sf::Vector2f Engine::screenToWorld(sf::Vector2i screenPos) {
    // Получаем view вьюпорта
    sf::View viewportView = m_Viewport.getView();

    float relativeX = (screenPos.x - m_viewportPos.x) / m_viewportSize.x;
    float relativeY = (screenPos.y - m_viewportPos.y) / m_viewportSize.y;

    // Проверяем, что точка внутри вьюпорта
    if (relativeX < 0 || relativeX > 1 || relativeY < 0 || relativeY > 1) {
        return { -1, -1 }; // Вне вьюпорта
    }

    // Получаем параметры view
    sf::Vector2f viewCenter = viewportView.getCenter();
    sf::Vector2f viewSize = viewportView.getSize();

    // Вычисляем мировые координаты
    float worldX = viewCenter.x + (relativeX - 0.5f) * viewSize.x;

    // Для Y учитываем инверсию
    float worldY;
    if (viewSize.y < 0) {
        // Если view инвертирован по Y (высота отрицательная)
        worldY = viewCenter.y - (relativeY - 0.5f) * viewSize.y;
    }
    else {
        // Обычный view
        worldY = viewCenter.y + (relativeY - 0.5f) * viewSize.y;
    }

    return { worldX, worldY };
}