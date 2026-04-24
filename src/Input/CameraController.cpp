#include "CameraController.hpp"

CameraController::CameraController(sf::RenderWindow& window, Viewport& viewport)
    : m_Window(window)
    , m_Viewport(viewport) {
}

void CameraController::handleEvent(const sf::Event& event) {
    // Средняя кнопка - ТОЛЬКО если мышь внутри вьюпорта
    if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseBtn->button == sf::Mouse::Button::Middle) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(m_Window);
            if (m_Viewport.isPointInside(mousePos)) {
                startDragging();
            }
        }
    }

    if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseBtn->button == sf::Mouse::Button::Middle) {
            stopDragging();
        }
    }

    // Колёсико - зум, только внутри вьюпорта
    if (const auto* mouseWheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(m_Window);
        if (m_Viewport.isPointInside(mousePos)) {
            handleZoom(mouseWheel->delta);
        }
    }
}


void CameraController::update() {
    if (!m_IsDragging) return;

    // Дополнительная проверка - если мышь вышла за пределы, прекращаем
    sf::Vector2i mousePos = sf::Mouse::getPosition(m_Window);
    if (!m_Viewport.isPointInside(mousePos)) {
        stopDragging();
        return;
    }

    sf::Vector2i currentPos = sf::Mouse::getPosition(m_Window);
    sf::Vector2i delta = currentPos - m_LastMousePos;

    if (delta.x != 0 || delta.y != 0) {
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
        // Зумим к позиции мыши
        sf::Vector2i mousePos = sf::Mouse::getPosition(m_Window);

        // Получаем мировые координаты до зума
        sf::Vector2f worldBefore = CoordinateConverter::screenToWorld(mousePos, m_Window, m_Viewport);

        m_Viewport.zoom(zoomFactor);

        // Получаем мировые координаты после зума
        sf::Vector2f worldAfter = CoordinateConverter::screenToWorld(mousePos, m_Window, m_Viewport);

        // Корректируем центр чтобы точка под мышью осталась на месте
        sf::Vector2f offset = worldBefore - worldAfter;
        m_Viewport.move(offset);
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