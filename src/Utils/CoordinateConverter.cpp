#include "CoordinateConverter.hpp"

//Конвертер координат
sf::Vector2f CoordinateConverter::screenToWorld(
    const sf::Vector2i& screenPos,
    const sf::RenderWindow& window,
    Viewport& viewport) {

    //Получаем позицию и размер
    sf::Vector2f viewportPos = viewport.getScreenPosition();
    sf::Vector2f viewportSize = viewport.getScreenSize();

    //Корректируем
    float relativeX = (screenPos.x - viewportPos.x) / viewportSize.x;
    float relativeY = (screenPos.y - viewportPos.y) / viewportSize.y;

    //Проверяем выход за границы
    if (relativeX < 0.f || relativeX > 1.f || relativeY < 0.f || relativeY > 1.f) {
        return { -1.f, -1.f };
    }

    //Устанавливаем
    sf::View viewportView = viewport.getView();
    sf::Vector2f viewCenter = viewportView.getCenter();
    sf::Vector2f viewSize = viewportView.getSize();

    float worldX = viewCenter.x + (relativeX - 0.5f) * viewSize.x;
    float worldY;

    //Проверка на отрицательный Y
    if (viewSize.y < 0) {
        worldY = viewCenter.y - (relativeY - 0.5f) * viewSize.y;
    }
    else {
        worldY = viewCenter.y + (relativeY - 0.5f) * viewSize.y;
    }

    return { worldX, worldY };
}


//Обратная функция
sf::Vector2i CoordinateConverter::worldToScreen(
    const sf::Vector2f& worldPos,
    const sf::RenderWindow& window,
    Viewport& viewport) {

    sf::View viewportView = viewport.getView();
    sf::Vector2f viewCenter = viewportView.getCenter();
    sf::Vector2f viewSize = viewportView.getSize();
    sf::Vector2f viewportPos = viewport.getScreenPosition();
    sf::Vector2f viewportSize = viewport.getScreenSize();

    float relativeX = (worldPos.x - viewCenter.x) / viewSize.x + 0.5f;
    float relativeY;

    if (viewSize.y < 0) {
        relativeY = (viewCenter.y - worldPos.y) / viewSize.y + 0.5f;
    }
    else {
        relativeY = (worldPos.y - viewCenter.y) / viewSize.y + 0.5f;
    }

    int screenX = static_cast<int>(viewportPos.x + relativeX * viewportSize.x);
    int screenY = static_cast<int>(viewportPos.y + relativeY * viewportSize.y);

    return { screenX, screenY };
}