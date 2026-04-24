#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "../ECS/Scene.hpp"

class Viewport {
public:
    Viewport(unsigned int width, unsigned int height);
    ~Viewport() = default;

    // Основные операции
    void clear(const sf::Color& color);
    void display();
    void draw(const sf::Drawable& drawable);

    // Управление view
    sf::View getView() const;
    void setView(const sf::View& view);
    void zoom(float factor);
    void move(const sf::Vector2f& offset);

    // Размеры
    sf::Vector2u getSize() const;
    void resize(const sf::Vector2u& size);

    // Доступ к текстуре
    sf::RenderTexture& getRenderTexture() { return m_RenderTexture; }
    const sf::Texture& getTexture() const { return m_RenderTexture.getTexture(); }

    // Позиционирование в окне
    void setScreenPosition(const sf::Vector2f& pos) { m_ScreenPosition = pos; }
    sf::Vector2f getScreenPosition() const { return m_ScreenPosition; }

    void setScreenSize(const sf::Vector2f& size) { m_ScreenSize = size; }
    sf::Vector2f getScreenSize() const { return m_ScreenSize; }

    // Проверки
    bool isPointInside(const sf::Vector2i& screenPos) const;
    void renderDebugOverlays(Scene& scene, Entity selectedEntity, const sf::Vector2f& mouseWorldPos);

    // Настройки отображения
    void setShowGrid(bool show) { m_ShowGrid = show; }
    bool isShowingGrid() const { return m_ShowGrid; }

    void setShowBounds(bool show) { m_ShowBounds = show; }
    bool isShowingBounds() const { return m_ShowBounds; }

    void setShowColliders(bool show) { m_ShowColliders = show; }
    bool isShowingColliders() const { return m_ShowColliders; }

    void setGridSize(float size) { m_GridSize = size; }
    float getGridSize() const { return m_GridSize; }

    // Режимы мыши
    enum class MouseMode {
        Select,
        Pan,
        Zoom
    };

    void setMouseMode(MouseMode mode) { m_MouseMode = mode; }
    MouseMode getMouseMode() const { return m_MouseMode; }

    bool isMouseInside() const { return m_MouseInside; }
    void setMouseInside(bool inside) { m_MouseInside = inside; }

private:
    sf::RenderTexture m_RenderTexture;
    sf::Vector2f m_ScreenPosition{ 0.f, 0.f };
    sf::Vector2f m_ScreenSize{ 0.f, 0.f };

    bool m_ShowGrid{ true };
    bool m_ShowBounds{ true };
    bool m_ShowColliders{ false };
    bool m_MouseInside{ false };
    float m_GridSize{ 100.f };
    sf::Color m_GridColor{ 60, 60, 60, 100 };

    MouseMode m_MouseMode{ MouseMode::Select };

    void drawGrid();
    void drawMousePosition(const sf::Vector2f& worldPos);
    void drawSelectionHighlight(Entity entity, Scene& scene);


};