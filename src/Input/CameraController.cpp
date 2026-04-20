#include "CameraController.hpp"

CameraController::CameraController(sf::RenderWindow& window, Viewport& viewport)
    : m_Window(window)
    , m_Viewport(viewport) {
}

void CameraController::handleEvent(const sf::Event& event) {
    // Средняя кнопка - перетаскивание
    if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseBtn->button == sf::Mouse::Button::Middle) {
            startDragging();
        }
    }

    if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseBtn->button == sf::Mouse::Button::Middle) {
            stopDragging();
        }
    }

    // Колёсико - зум
    if (const auto* mouseWheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        handleZoom(mouseWheel->delta);
    }
}

void CameraController::update() {
    if (!m_IsDragging) return;

    sf::Vector2i currentPos = sf::Mouse::getPosition(m_Window);
    sf::Vector2i delta = currentPos - m_LastMousePos;

    if (delta.x != 0 || delta.y != 0) {
        // Инвертируем движение для естественного перетаскивания
        m_Viewport.move({ static_cast<float>(-delta.x), static_cast<float>(-delta.y) });
        m_LastMousePos = currentPos;
    }
}

void CameraController::startDragging() {
    m_IsDragging = true;
    m_LastMousePos = sf::Mouse::getPosition(m_Window);
    m_Window.setMouseCursorVisible(false);
}

void CameraController::stopDragging() {
    m_IsDragging = false;
    m_Window.setMouseCursorVisible(true);
}

void CameraController::handleZoom(float delta) {
    float zoomFactor = 1.f - (delta * m_ZoomSpeed);
    float newZoom = m_Zoom * zoomFactor;

    if (newZoom >= m_MinZoom && newZoom <= m_MaxZoom) {
        m_Zoom = newZoom;
        m_Viewport.zoom(zoomFactor);
    }
}

void CameraController::reset() {
    sf::View view = m_Viewport.getView();
    auto size = m_Viewport.getSize();
    view.setCenter({ static_cast<float>(size.x) / 2.f, static_cast<float>(size.y) / 2.f });
    view.setSize({ static_cast<float>(size.x), -static_cast<float>(size.y) });
    m_Viewport.setView(view);
    m_Zoom = 1.f;
}

void CameraController::setZoom(float zoom) {
    if (zoom >= m_MinZoom && zoom <= m_MaxZoom) {
        float factor = zoom / m_Zoom;
        m_Viewport.zoom(factor);
        m_Zoom = zoom;
    }
}