#include "Viewport.hpp"
#include <cmath>

//Инициализация (установка размеров)
Viewport::Viewport(unsigned int width, unsigned int height) {
    m_RenderTexture.resize({ width, height });
    m_ScreenSize = { static_cast<float>(width), static_cast<float>(height) };
}

//Очистка (заливка цветом)
void Viewport::clear(const sf::Color& color) {
    m_RenderTexture.clear(color);
}

//Завершение отрисовки
void Viewport::display() {
    m_RenderTexture.display();
}

//Отрисовка
void Viewport::draw(const sf::Drawable& drawable) {
    m_RenderTexture.draw(drawable);
}

//Получение камеры
sf::View Viewport::getView() const {
    return m_RenderTexture.getView();
}

//Установка камеры
void Viewport::setView(const sf::View& view) {
    m_RenderTexture.setView(view);
}

//Установка зума
void Viewport::zoom(float factor) {
    sf::View view = getView();
    view.zoom(factor);
    setView(view);
}

//Передвижение камеры
void Viewport::move(const sf::Vector2f& offset) {
    sf::View view = getView();
    view.move(offset);
    setView(view);
}

//Размер
sf::Vector2u Viewport::getSize() const {
    return m_RenderTexture.getSize();
}

//Изменение размера
void Viewport::resize(const sf::Vector2u& size) {
    m_RenderTexture.resize(size);
}

//Проверка позиции
bool Viewport::isPointInside(const sf::Vector2i& screenPos) const {
    return screenPos.x >= m_ScreenPosition.x &&
        screenPos.x <= m_ScreenPosition.x + m_ScreenSize.x &&
        screenPos.y >= m_ScreenPosition.y &&
        screenPos.y <= m_ScreenPosition.y + m_ScreenSize.y;
}

//Отрисовка Debug (ДОБАВИТЬ В ФУНКЦИЮ MODE)
void Viewport::renderDebugOverlays(Scene& scene, Entity selectedEntity, const sf::Vector2f& mouseWorldPos) {
    // Рисуем границы всех спрайтов
    for (auto& [entity, spriteComp] : scene.sprites) {
        if (!spriteComp.sprite) continue;

        sf::FloatRect bounds = spriteComp.sprite->getGlobalBounds();

        // Зелёный хитбокс(текстура)
        sf::RectangleShape hitbox;
        hitbox.setPosition({ bounds.position.x, bounds.position.y });
        hitbox.setSize({ bounds.size.x, bounds.size.y });
        hitbox.setFillColor(sf::Color(0, 255, 0, 30));
        hitbox.setOutlineColor(sf::Color::Green);
        hitbox.setOutlineThickness(1.f);
        m_RenderTexture.draw(hitbox);

        // Подсветка при наведении
        if (bounds.contains(mouseWorldPos)) {
            sf::RectangleShape hoverHighlight;
            hoverHighlight.setPosition({ bounds.position.x, bounds.position.y });
            hoverHighlight.setSize({ bounds.size.x, bounds.size.y });
            hoverHighlight.setFillColor(sf::Color(255, 255, 0, 40));
            hoverHighlight.setOutlineColor(sf::Color::Yellow);
            hoverHighlight.setOutlineThickness(2.f);
            m_RenderTexture.draw(hoverHighlight);
        }
    }

    // Выделение выбранной сущности
    if (selectedEntity != INVALID_ENTITY) {
        auto spriteIt = scene.sprites.find(selectedEntity);
        if (spriteIt != scene.sprites.end() && spriteIt->second.sprite) {
            sf::FloatRect bounds = spriteIt->second.sprite->getGlobalBounds();

            // Белая рамка
            sf::RectangleShape selectionRect;
            selectionRect.setPosition({ bounds.position.x, bounds.position.y });
            selectionRect.setSize({ bounds.size.x, bounds.size.y });
            selectionRect.setFillColor(sf::Color::Transparent);
            selectionRect.setOutlineColor(sf::Color::White);
            selectionRect.setOutlineThickness(3.f);
            m_RenderTexture.draw(selectionRect);

            // Точка трансформа
            auto tfIt = scene.transforms.find(selectedEntity);
            if (tfIt != scene.transforms.end()) {
                sf::CircleShape transformPoint(5.f);
                transformPoint.setPosition(tfIt->second.Pos);
                transformPoint.setOrigin({ 5.f, 5.f });
                transformPoint.setFillColor(sf::Color::Blue);
                m_RenderTexture.draw(transformPoint);
            }
        }
    }

    // Курсор мыши
    drawMousePosition(mouseWorldPos);
}

//Отрисовка мыши (Также добавить)
void Viewport::drawMousePosition(const sf::Vector2f& worldPos) {
    // Точка
    sf::CircleShape point(4.f);
    point.setPosition(worldPos);
    point.setOrigin({ 4.f, 4.f });
    point.setFillColor(sf::Color::Magenta);
    m_RenderTexture.draw(point);

    // Крестик
    sf::VertexArray cross(sf::PrimitiveType::Lines, 4);
    cross[0].position = { worldPos.x - 20.f, worldPos.y };
    cross[1].position = { worldPos.x + 20.f, worldPos.y };
    cross[2].position = { worldPos.x, worldPos.y - 20.f };
    cross[3].position = { worldPos.x, worldPos.y + 20.f };

    sf::Color crossColor = sf::Color(255, 0, 255, 200);
    cross[0].color = cross[1].color = cross[2].color = cross[3].color = crossColor;

    m_RenderTexture.draw(cross);
}