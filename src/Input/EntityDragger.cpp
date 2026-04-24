#include "EntityDragger.hpp"
#include "../Utils/CoordinateConverter.hpp"
#include <vector>
#include <algorithm>

EntityDragger::EntityDragger(sf::RenderWindow& window, Viewport& viewport)
    : m_Window(window)
    , m_Viewport(viewport) {
}

//Позиция мыши
sf::Vector2f EntityDragger::getMouseWorldPos() const {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(m_Window);
    return CoordinateConverter::screenToWorld(pixelPos, m_Window, m_Viewport);
}

//Проверка на обьект
Entity EntityDragger::tryGrabEntity(Scene& scene, const sf::Vector2f& worldPos) {
    // Собираем все спрайты в вектор для сортировки по слоям
    std::vector<std::pair<Entity, SpriteComponent*>> sortedSprites;
    for (auto& [entity, spriteComp] : scene.sprites) {
        sortedSprites.push_back({ entity, &spriteComp });
    }

    // Сортируем по слоям (от верхних к нижним)
    std::sort(sortedSprites.begin(), sortedSprites.end(),
        [](const auto& a, const auto& b) {
            return a.second->layer > b.second->layer;  // Сначала верхние слои
        });

    // Ищем спрайт под курсором
    for (const auto& [entity, spriteCompPtr] : sortedSprites) {
        if (!spriteCompPtr->sprite) continue;

        sf::FloatRect bounds = spriteCompPtr->sprite->getGlobalBounds();

        if (bounds.contains(worldPos)) {
            m_IsDragging = true;
            m_DraggedEntity = entity;

            auto tfIt = scene.transforms.find(entity);
            if (tfIt != scene.transforms.end()) {
                m_DragOffset.x = tfIt->second.Pos.x - worldPos.x;
                m_DragOffset.y = tfIt->second.Pos.y - worldPos.y;
            }

            return entity;
        }
    }

    return INVALID_ENTITY;
}

void EntityDragger::update(Scene& scene) {
    if (!m_IsDragging || m_DraggedEntity == INVALID_ENTITY) return;

    sf::Vector2f worldPos = getMouseWorldPos();

    // Проверяем что мышь внутри вьюпорта
    sf::Vector2i pixelPos = sf::Mouse::getPosition(m_Window);
    if (!m_Viewport.isPointInside(pixelPos)) {
        release();
        return;
    }

    auto tfIt = scene.transforms.find(m_DraggedEntity);
    if (tfIt != scene.transforms.end()) {
        tfIt->second.Pos.x = worldPos.x + m_DragOffset.x;
        tfIt->second.Pos.y = worldPos.y + m_DragOffset.y;
    }
}

void EntityDragger::release() {
    m_IsDragging = false;
    m_DraggedEntity = INVALID_ENTITY;
}