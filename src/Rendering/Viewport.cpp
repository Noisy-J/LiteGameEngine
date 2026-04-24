#include "Viewport.hpp"
#include <cmath>

Viewport::Viewport(unsigned int width, unsigned int height) {
    m_RenderTexture.resize({ width, height });
    m_ScreenSize = { static_cast<float>(width), static_cast<float>(height) };
}

void Viewport::clear(const sf::Color& color) {
    m_RenderTexture.clear(color);

    if (m_ShowGrid) {
        drawGrid();
    }
}

void Viewport::display() {
    m_RenderTexture.display();
}

void Viewport::draw(const sf::Drawable& drawable) {
    m_RenderTexture.draw(drawable);
}

sf::View Viewport::getView() const {
    return m_RenderTexture.getView();
}

void Viewport::setView(const sf::View& view) {
    m_RenderTexture.setView(view);
}

void Viewport::zoom(float factor) {
    sf::View view = getView();
    view.zoom(factor);
    setView(view);
}

void Viewport::move(const sf::Vector2f& offset) {
    sf::View view = getView();
    view.move(offset);
    setView(view);
}

sf::Vector2u Viewport::getSize() const {
    return m_RenderTexture.getSize();
}

void Viewport::resize(const sf::Vector2u& size) {
    m_RenderTexture.resize(size);
}

bool Viewport::isPointInside(const sf::Vector2i& screenPos) const {
    return screenPos.x >= m_ScreenPosition.x &&
        screenPos.x <= m_ScreenPosition.x + m_ScreenSize.x &&
        screenPos.y >= m_ScreenPosition.y &&
        screenPos.y <= m_ScreenPosition.y + m_ScreenSize.y;
}

void Viewport::drawGrid() {
    sf::View view = getView();
    sf::Vector2f viewCenter = view.getCenter();
    sf::Vector2f viewSize = view.getSize();

    float gridSize = m_GridSize;

    float left = viewCenter.x - std::abs(viewSize.x) / 2.f;
    float right = viewCenter.x + std::abs(viewSize.x) / 2.f;
    float top = viewCenter.y - std::abs(viewSize.y) / 2.f;
    float bottom = viewCenter.y + std::abs(viewSize.y) / 2.f;

    // Вертикальные линии
    float startX = std::floor(left / gridSize) * gridSize;
    for (float x = startX; x <= right; x += gridSize) {
        sf::Vertex line[] = {
            sf::Vertex({x, top}, m_GridColor),
            sf::Vertex({x, bottom}, m_GridColor)
        };
        m_RenderTexture.draw(line, 2, sf::PrimitiveType::Lines);
    }

    // Горизонтальные линии
    float startY = std::floor(top / gridSize) * gridSize;
    for (float y = startY; y <= bottom; y += gridSize) {
        sf::Vertex line[] = {
            sf::Vertex({left, y}, m_GridColor),
            sf::Vertex({right, y}, m_GridColor)
        };
        m_RenderTexture.draw(line, 2, sf::PrimitiveType::Lines);
    }

    // Оси
    sf::Color axisColor(80, 80, 80, 150);
    sf::Vertex xAxis[] = {
        sf::Vertex({left, 0.f}, axisColor),
        sf::Vertex({right, 0.f}, axisColor)
    };
    sf::Vertex yAxis[] = {
        sf::Vertex({0.f, top}, axisColor),
        sf::Vertex({0.f, bottom}, axisColor)
    };
    m_RenderTexture.draw(xAxis, 2, sf::PrimitiveType::Lines);
    m_RenderTexture.draw(yAxis, 2, sf::PrimitiveType::Lines);
}

void Viewport::renderDebugOverlays(Scene& scene, Entity selectedEntity, const sf::Vector2f& mouseWorldPos) {
    // Границы спрайтов
    if (m_ShowBounds) {
        for (auto& [entity, spriteComp] : scene.sprites) {
            if (!spriteComp.sprite) continue;

            sf::FloatRect bounds = spriteComp.sprite->getGlobalBounds();

            sf::RectangleShape hitbox;
            hitbox.setPosition({ bounds.position.x, bounds.position.y });
            hitbox.setSize({ bounds.size.x, bounds.size.y });
            hitbox.setFillColor(sf::Color(0, 255, 0, 20));
            hitbox.setOutlineColor(sf::Color::Green);
            hitbox.setOutlineThickness(1.f);
            m_RenderTexture.draw(hitbox);

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
    }

    // Коллайдеры
    if (m_ShowColliders) {
        for (auto& [entity, collider] : scene.colliders) {
            auto tfIt = scene.transforms.find(entity);
            if (tfIt == scene.transforms.end()) continue;

            auto& tf = tfIt->second;

            if (collider.shape == ColliderComponent::Shape::Box) {
                sf::RectangleShape box;
                box.setPosition(tf.Pos + collider.offset - collider.size / 2.f);
                box.setSize(collider.size);
                box.setFillColor(sf::Color(0, 100, 255, 30));
                box.setOutlineColor(sf::Color::Blue);
                box.setOutlineThickness(1.f);
                m_RenderTexture.draw(box);
            }
            else {
                sf::CircleShape circle(collider.radius);
                circle.setPosition(tf.Pos + collider.offset);
                circle.setOrigin({ collider.radius, collider.radius });
                circle.setFillColor(sf::Color(0, 100, 255, 30));
                circle.setOutlineColor(sf::Color::Blue);
                circle.setOutlineThickness(1.f);
                m_RenderTexture.draw(circle);
            }
        }
    }

    // Выделение выбранной сущности
    if (selectedEntity != INVALID_ENTITY) {
        drawSelectionHighlight(selectedEntity, scene);
    }

    // Курсор мыши
    drawMousePosition(mouseWorldPos);
}

void Viewport::drawSelectionHighlight(Entity entity, Scene& scene) {
    auto tfIt = scene.transforms.find(entity);
    if (tfIt == scene.transforms.end()) return;

    auto& tf = tfIt->second;

    // Точка позиции
    sf::CircleShape posPoint(5.f);
    posPoint.setPosition(tf.Pos);
    posPoint.setOrigin({ 5.f, 5.f });
    posPoint.setFillColor(sf::Color::Blue);
    m_RenderTexture.draw(posPoint);

    // Если есть спрайт - выделяем его
    auto spriteIt = scene.sprites.find(entity);
    if (spriteIt != scene.sprites.end() && spriteIt->second.sprite) {
        sf::FloatRect bounds = spriteIt->second.sprite->getGlobalBounds();

        sf::RectangleShape selectionRect;
        selectionRect.setPosition({ bounds.position.x, bounds.position.y });
        selectionRect.setSize({ bounds.size.x, bounds.size.y });
        selectionRect.setFillColor(sf::Color::Transparent);
        selectionRect.setOutlineColor(sf::Color::White);
        selectionRect.setOutlineThickness(2.f);
        m_RenderTexture.draw(selectionRect);

        // Угловые маркеры
        float markerSize = 8.f;
        sf::RectangleShape marker;
        marker.setSize({ markerSize, markerSize });
        marker.setFillColor(sf::Color::White);

        marker.setPosition({ bounds.position.x - markerSize / 2, bounds.position.y - markerSize / 2 });
        m_RenderTexture.draw(marker);
        marker.setPosition({ bounds.position.x + bounds.size.x - markerSize / 2, bounds.position.y - markerSize / 2 });
        m_RenderTexture.draw(marker);
        marker.setPosition({ bounds.position.x - markerSize / 2, bounds.position.y + bounds.size.y - markerSize / 2 });
        m_RenderTexture.draw(marker);
        marker.setPosition({ bounds.position.x + bounds.size.x - markerSize / 2, bounds.position.y + bounds.size.y - markerSize / 2 });
        m_RenderTexture.draw(marker);
    }
}

void Viewport::drawMousePosition(const sf::Vector2f& worldPos) {
    sf::CircleShape point(3.f);
    point.setPosition(worldPos);
    point.setOrigin({ 3.f, 3.f });
    point.setFillColor(sf::Color::Magenta);
    m_RenderTexture.draw(point);

    sf::VertexArray cross(sf::PrimitiveType::Lines, 4);
    cross[0].position = { worldPos.x - 15.f, worldPos.y };
    cross[1].position = { worldPos.x + 15.f, worldPos.y };
    cross[2].position = { worldPos.x, worldPos.y - 15.f };
    cross[3].position = { worldPos.x, worldPos.y + 15.f };

    sf::Color crossColor(255, 0, 255, 150);
    cross[0].color = cross[1].color = cross[2].color = cross[3].color = crossColor;
    m_RenderTexture.draw(cross);
}