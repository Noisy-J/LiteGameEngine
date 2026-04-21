#pragma once

#include "Entity.hpp"
#include "Components.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

class Scene {
public:
    Scene() = default;
    ~Scene() = default;

    // Управление сущностями
    Entity createEntity();
    void destroyEntity(Entity entity);
    bool isValid(Entity entity) const;

    // Transform (всегда есть)
    void setPosition(Entity entity, const sf::Vector2f& pos);
    sf::Vector2f getPosition(Entity entity) const;
    void setRotation(Entity entity, sf::Angle angle);
    sf::Angle getRotation(Entity entity) const;
    void setScale(Entity entity, const sf::Vector2f& scale);
    sf::Vector2f getScale(Entity entity) const;

    // Sprite
    void setTexture(Entity entity, std::shared_ptr<sf::Texture> texture, const std::string& path = "");
    const sf::Texture* getTexture(Entity entity) const;
    sf::Sprite* getSprite(Entity entity);

    // Velocity
    void setVelocity(Entity entity, const sf::Vector2f& velocity);
    sf::Vector2f getVelocity(Entity entity) const;

    // Доступ к данным (для систем)
    std::unordered_map<Entity, TransformComponent> transforms;
    std::unordered_map<Entity, SpriteComponent> sprites;
    std::unordered_map<Entity, VelocityComponent> velocities;
    std::unordered_map<Entity, TagComponent> tags;
    std::unordered_map<Entity, HealthComponent> healths;
    std::unordered_map<Entity, ColliderComponent> colliders;
    std::unordered_map<Entity, ScriptComponent> scripts;

private:
    Entity m_NextEntity{ 0 };
    std::vector<Entity> m_FreeEntities;
};