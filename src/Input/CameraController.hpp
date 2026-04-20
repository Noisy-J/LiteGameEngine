#pragma once

#include <SFML/Graphics.hpp>
#include "../Rendering/Viewport.hpp"

class CameraController {
public:
    CameraController(sf::RenderWindow& window, Viewport& viewport);
    ~CameraController() = default;

    void handleEvent(const sf::Event& event);
    void update();

    void reset();
    void setZoom(float zoom);
    float getZoom() const { return m_Zoom; }

private:
    sf::RenderWindow& m_Window;
    Viewport& m_Viewport;

    bool m_IsDragging{ false };
    sf::Vector2i m_LastMousePos{ 0, 0 };
    float m_Zoom{ 1.f };
    float m_ZoomSpeed{ 0.1f };
    float m_MinZoom{ 0.1f };
    float m_MaxZoom{ 5.f };

    void startDragging();
    void stopDragging();
    void handleZoom(float delta);
};