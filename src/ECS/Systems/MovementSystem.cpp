#include "MovementSystem.hpp"

void MovementSystem::update(Scene& scene, float deltaTime) {
    for (auto& [entity, transform] : scene.transforms) {
        auto velIt = scene.velocities.find(entity);
        if (velIt == scene.velocities.end()) continue;

        auto& velocity = velIt->second;

        // Применяем скорость
        transform.Pos.x += velocity.Velocity.x * deltaTime;
        transform.Pos.y += velocity.Velocity.y * deltaTime;

        // Применяем damping
        if (velocity.damping > 0.f) {
            velocity.Velocity.x *= (1.f - velocity.damping * deltaTime);
            velocity.Velocity.y *= (1.f - velocity.damping * deltaTime);

            // Останавливаем если скорость очень маленькая
            if (std::abs(velocity.Velocity.x) < 0.1f) velocity.Velocity.x = 0.f;
            if (std::abs(velocity.Velocity.y) < 0.1f) velocity.Velocity.y = 0.f;
        }

        // Ограничение максимальной скорости
        if (velocity.maxSpeed > 0.f) {
            float speed = std::sqrt(velocity.Velocity.x * velocity.Velocity.x +
                velocity.Velocity.y * velocity.Velocity.y);
            if (speed > velocity.maxSpeed) {
                velocity.Velocity.x = (velocity.Velocity.x / speed) * velocity.maxSpeed;
                velocity.Velocity.y = (velocity.Velocity.y / speed) * velocity.maxSpeed;
            }
        }
    }
}