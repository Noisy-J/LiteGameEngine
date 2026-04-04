#include "Engine.hpp"

Engine::Engine() {

    // Создаем окно (SFML 3.0)
    m_Window.create(sf::VideoMode({ 800, 600 }), "Lite Game Engine v0.2");
    m_Window.setFramerateLimit(60);

    // Инициализация ImGui-SFML
    if (ImGui::SFML::Init(m_Window)) {

        //Текстура шрифтов
        ImGui::SFML::UpdateFontTexture();
    }
}

void Engine::run() {

    //sf::RectangleShape sprite(sf::Vector2f(100, 100));
    sf::Texture texture;

    if (!texture.loadFromFile("./assets/hero.png")) {
        //return -1;
    }

    sf::Sprite sprite(texture);
    //sprite.setTexture(texture);

    sf::Vector2f pos(100, 100);
    sprite.setPosition(pos);

    while (m_Window.isOpen()) {
        sf::Time dtTime = m_Clock.restart();

        input();
        update(dtTime, sprite);
        draw(sprite);
    }
}

void Engine::input() {

    // Обработка событий (совместимо с SFML 3.0 std::optional)
    while (const std::optional<sf::Event> event = m_Window.pollEvent()) {

        // Передаем событие в ImGui
        ImGui::SFML::ProcessEvent(m_Window, *event);

        // Закрытие окна
        if (event->is<sf::Event::Closed>()) {
            m_Window.close();
        }
    }
}

void Engine::update(sf::Time dt, sf::Sprite sprite) {

    // Начинаем новый кадр ImGui
    ImGui::SFML::Update(m_Window, dt);

    // Окно настроек (Debug Panel)
    ImGui::Begin("Debug Panel");
    ImGui::Text("Position \nX: %f | Y: %f", 
        sprite.getPosition().x, sprite.getPosition().y);

    static int counter = 0;

    if (ImGui::Button("Click Me")) {
        counter++;
    }
    ImGui::Text("Counter: %d", counter);

    ImGui::End();
}

void Engine::draw(sf::Sprite sprite) {
    
    m_Window.clear(sf::Color(30, 30, 30)); // Темно-серый фон

    m_Window.draw(sprite);

    // Рендерим ImGui в самом конце, поверх всего остального
    ImGui::SFML::Render(m_Window);

    m_Window.display();
}
