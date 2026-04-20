#pragma once

#include <SFML/Graphics.hpp>
#include "../ECS/Scene.hpp"
#include "../Rendering/Viewport.hpp"

class EntityDragger {
public:
    EntityDragger(sf::RenderWindow& window, Viewport& viewport);
    ~EntityDragger() = default;

    
    Entity tryGrabEntity(Scene& scene, const sf::Vector2f& worldPos);
    void update(Scene& scene);
    void release();

    bool isDragging() const { return m_IsDragging; }
    Entity getDraggedEntity() const { return m_DraggedEntity; }

private:
    sf::RenderWindow& m_Window;
    Viewport& m_Viewport;

    bool m_IsDragging{ false };
    Entity m_DraggedEntity{ INVALID_ENTITY };
    sf::Vector2f m_DragOffset{ 0.f, 0.f };

    sf::Vector2f getMouseWorldPos() const;
};