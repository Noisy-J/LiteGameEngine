#pragma once

#include <string>
#include <filesystem>
#include "../ECS/Scene.hpp"

class SceneSerializer {
public:
    SceneSerializer(Scene& scene);

    void newScene();
    bool save(const std::string& filepath);
    bool load(const std::string& filepath);

private:
    Scene& m_Scene;

    std::string serializeTransform(Entity entity);
    std::string serializeSprite(Entity entity);
    std::string serializeVelocity(Entity entity);
    std::string serializeCollider(Entity entity);
    std::string serializeHealth(Entity entity);
    std::string serializeTag(Entity entity);

    void deserializeTransform(Entity entity, const std::string& json);
    void deserializeSprite(Entity entity, const std::string& json);
    void deserializeVelocity(Entity entity, const std::string& json);
    void deserializeCollider(Entity entity, const std::string& json);
    void deserializeHealth(Entity entity, const std::string& json);
    void deserializeTag(Entity entity, const std::string& json);
};