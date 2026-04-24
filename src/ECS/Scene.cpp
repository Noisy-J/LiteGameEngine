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

    // Всегда добавляем Transform и Name
    transforms[entity] = TransformComponent{};
    names[entity].Name = "Entity " + std::to_string(entity);

    return entity;
}

void Scene::destroyEntity(Entity entity) {
    if (!isValid(entity)) {
        std::cerr << "destroyEntity: Invalid entity " << entity << std::endl;
        return;
    }

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
void Scene::setTexture(Entity entity, std::shared_ptr<sf::Texture> texture, const std::string& path) {
    auto& spriteComp = sprites[entity];
    spriteComp.texture = texture;
    spriteComp.texturePath = path;  // Сохраняем путь

    if (!spriteComp.sprite) {
        spriteComp.sprite = std::make_unique<sf::Sprite>(*texture);
    }
    else {
        spriteComp.sprite->setTexture(*texture);
    }

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

void Scene::setParent(Entity child, Entity parent) {
    if (child == parent) return; // Нельзя быть родителем самого себя
    if (!isValid(child) || !isValid(parent)) return;

    // Удаляем из старого родителя
    if (parents.find(child) != parents.end()) {
        Entity oldParent = parents[child].Parent;
        if (children.find(oldParent) != children.end()) {
            auto& oldChildren = children[oldParent].Children;
            oldChildren.erase(std::remove(oldChildren.begin(), oldChildren.end(), child), oldChildren.end());
        }
    }

    // Устанавливаем нового родителя
    parents[child].Parent = parent;
    if (children.find(parent) == children.end()) {
        children[parent] = ChildrenComponent{};
    }
    children[parent].Children.push_back(child);
}

void Scene::removeParent(Entity child) {
    if (parents.find(child) != parents.end()) {
        Entity parent = parents[child].Parent;
        if (children.find(parent) != children.end()) {
            auto& childList = children[parent].Children;
            childList.erase(std::remove(childList.begin(), childList.end(), child), childList.end());
        }
        parents.erase(child);
    }
}

std::vector<Entity> Scene::getRootEntities() const {
    std::vector<Entity> roots;
    for (const auto& [entity, _] : transforms) {
        if (parents.find(entity) == parents.end() || parents.at(entity).Parent == INVALID_ENTITY) {
            roots.push_back(entity);
        }
    }
    return roots;
}

std::vector<Entity> Scene::getChildren(Entity entity) const {
    if (children.find(entity) != children.end()) {
        return children.at(entity).Children;
    }
    return {};
}

Entity Scene::getParent(Entity entity) const {
    auto it = parents.find(entity);
    if (it != parents.end()) {
        return it->second.Parent;
    }
    return INVALID_ENTITY;
}

void Scene::setEntityName(Entity entity, const std::string& name) {
    names[entity].Name = name;
}

std::string Scene::getEntityName(Entity entity) const {
    auto it = names.find(entity);
    if (it != names.end()) {
        return it->second.Name;
    }
    return "Entity " + std::to_string(entity);
}
