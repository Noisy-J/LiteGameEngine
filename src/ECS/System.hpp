#pragma once
#include "Scene.hpp"
#include <iostream>

class MovementSystem {
public:
    static void update(Scene& scene, float dt) {
        for (auto& [entity, vel] : scene.velocities) {
            auto it = scene.transforms.find(entity);
            if (it != scene.transforms.end()) {
                it->second.Pos += vel.Velocity * dt;
            }
        }
    }
};
class RenderSystem{
public:
    static void drawSprites(Scene& scene, sf::RenderTarget& target) {
        for (auto& [entity, spriteComp] : scene.sprites) {
            // Проверяем, создан ли спрайт внутри компонента
            if (spriteComp.sprite.has_value()) {
                auto it = scene.transforms.find(entity);
                if (it != scene.transforms.end()) {
                    auto& tf = it->second;

                    // Доступ через -> так как это optional
                    spriteComp.sprite->setPosition(tf.Pos);
                    spriteComp.sprite->setRotation(tf.Rot);
                    spriteComp.sprite->setScale(tf.Scale);

                    target.draw(*spriteComp.sprite);
                }
            }
        }
    }
};