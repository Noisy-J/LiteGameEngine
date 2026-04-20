#include "RenderSystem.hpp"
#include <algorithm>
#include <vector>

void RenderSystem::drawSprites(Scene& scene, sf::RenderTarget& target) {
    // Собираем спрайты для сортировки по слоям
    std::vector<std::pair<int, sf::Sprite*>> spritesToDraw;

    for (auto& [entity, spriteComp] : scene.sprites) {
        if (!spriteComp.visible || !spriteComp.sprite) continue;

        auto tfIt = scene.transforms.find(entity);
        if (tfIt == scene.transforms.end()) continue;

        auto& transform = tfIt->second;
        auto& sprite = spriteComp.sprite;

        // Применяем трансформации
        sprite->setPosition(transform.Pos);
        sprite->setRotation(transform.Rot);
        sprite->setScale(transform.Scale);

        spritesToDraw.push_back({ spriteComp.layer, sprite.get() });
    }

    // Сортируем по слоям (от заднего к переднему)
    std::sort(spritesToDraw.begin(), spritesToDraw.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        });

    // Рисуем
    for (const auto& [layer, sprite] : spritesToDraw) {
        target.draw(*sprite);
    }
}