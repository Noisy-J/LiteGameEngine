#include "Scene.hpp"
#include <iostream>

Entity Scene::createEntity() {
    Entity entity;

    if (!m_FreeEntities.empty()) {
        entity = m_FreeEntities.back();
        m_FreeEntities.pop_back();
    }
    else {
        entity = m_NextEntity++;
    }

    // Каждая сущность всегда имеет Transform
    transforms[entity] = TransformComponent{};

    return entity;
}

void Scene::destroyEntity(Entity entity) {
    if (!isValid(entity)) return;

    transforms.erase(entity);
    sprites.erase(entity);
    velocities.erase(entity);
    tags.erase(entity);
    healths.erase(entity);
    colliders.erase(entity);
    scripts.erase(entity);

    m_FreeEntities.push_back(entity);
}

bool Scene::isValid(Entity entity) const {
    return transforms.find(entity) != transforms.end();
}

// Transform
void Scene::setPosition(Entity entity, const sf::Vector2f& pos) {
    if (auto it = transforms.find(entity); it != transforms.end()) {
        it->second.Pos = pos;
    }
}

sf::Vector2f Scene::getPosition(Entity entity) const {
    if (auto it = transforms.find(entity); it != transforms.end()) {
        return it->second.Pos;
    }
    return { 0.f, 0.f };
}

void Scene::setRotation(Entity entity, sf::Angle angle) {
    if (auto it = transforms.find(entity); it != transforms.end()) {
        it->second.Rot = angle;
    }
}

sf::Angle Scene::getRotation(Entity entity) const {
    if (auto it = transforms.find(entity); it != transforms.end()) {
        return it->second.Rot;
    }
    return sf::degrees(0.f);
}

void Scene::setScale(Entity entity, const sf::Vector2f& scale) {
    if (auto it = transforms.find(entity); it != transforms.end()) {
        it->second.Scale = scale;
    }
}

sf::Vector2f Scene::getScale(Entity entity) const {
    if (auto it = transforms.find(entity); it != transforms.end()) {
        return it->second.Scale;
    }
    return { 1.f, 1.f };
}

// Sprite
void Scene::setTexture(Entity entity, std::shared_ptr<sf::Texture> texture) {
    auto& spriteComp = sprites[entity];
    spriteComp.texture = texture;

    if (!spriteComp.sprite) {
        // Вместо make_unique создаём через new с параметром
        spriteComp.sprite = std::unique_ptr<sf::Sprite>(new sf::Sprite(*texture));
    }
    else {
        spriteComp.sprite->setTexture(*texture);
    }

    // Центрируем origin
    sf::Vector2u texSize = texture->getSize();
    spriteComp.sprite->setOrigin({
        static_cast<float>(texSize.x) / 2.f,
        static_cast<float>(texSize.y) / 2.f
        });
}

const sf::Texture* Scene::getTexture(Entity entity) const {
    if (auto it = sprites.find(entity); it != sprites.end()) {
        return it->second.texture.get();
    }
    return nullptr;
}

sf::Sprite* Scene::getSprite(Entity entity) {
    if (auto it = sprites.find(entity); it != sprites.end()) {
        return it->second.sprite.get();  // Исправлено: .get() вместо .get()
    }
    return nullptr;
}

// Velocity
void Scene::setVelocity(Entity entity, const sf::Vector2f& velocity) {
    velocities[entity].Velocity = velocity;
}

sf::Vector2f Scene::getVelocity(Entity entity) const {
    if (auto it = velocities.find(entity); it != velocities.end()) {
        return it->second.Velocity;
    }
    return { 0.f, 0.f };
}

// Collider
void Scene::setColliderPosition(Entity entity, const sf::Vector2f& pos) {
    colliders[entity].ColliderPosition = pos;
}

sf::Vector2f Scene::getColliderPosition(Entity entity) const {
    if (auto it = colliders.find(entity); it != colliders.end()) {
        return it->second.ColliderPosition;
    }
    return { 0.f, 0.f };
}

void Scene::setColliderRotation(Entity entity, const sf::Angle angle) {
    colliders[entity].ColliderRotation = angle;
}

sf::Angle Scene::getColliderRotation(Entity entity) const {
    if (auto it = colliders.find(entity); it != colliders.end()) {
        return it->second.ColliderRotation;
    }
    return sf::degrees(0.f);
}

void Scene::setColliderScale(Entity entity, const sf::Vector2f& scale) {
    colliders[entity].ColliderScale = scale;
}

sf::Vector2f Scene::getColliderScale(Entity entity) const {
    if (auto it = colliders.find(entity); it != colliders.end()) {
        return it->second.ColliderScale;
    }
    return { 0.f, 0.f };
}

void Scene::setColliderOrigin(Entity entity, const sf::Vector2f& origin) {
    colliders[entity].ColliderScale = origin;
}

sf::Vector2f Scene::getColliderOrigin(Entity entity) const {
    if (auto it = colliders.find(entity); it != colliders.end()) {
        return it->second.ColliderOrigin;
    }
    return { 0.f, 0.f };
}

void Scene::setColliderSize(Entity entity, const sf::Vector2f& size) {
    colliders[entity].ColliderScale = size;
}

sf::Vector2f Scene::getColliderSize(Entity entity) const {
    if (auto it = colliders.find(entity); it != colliders.end()) {
        return it->second.ColliderSize;
    }
    return { 0.f, 0.f };
}

