#include "SceneSerializer.hpp"
#include "../Resources/ResourceManager.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <map>

namespace fs = std::filesystem;

// Простой JSON writer
class SimpleJson {
public:
    static std::string object(const std::vector<std::pair<std::string, std::string>>& fields) {
        std::stringstream ss;
        ss << "{";
        for (size_t i = 0; i < fields.size(); ++i) {
            ss << "\"" << fields[i].first << "\":" << fields[i].second;
            if (i < fields.size() - 1) ss << ",";
        }
        ss << "}";
        return ss.str();
    }

    static std::string string(const std::string& str) {
        return "\"" + escape(str) + "\"";
    }

    static std::string number(float num) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(3) << num;
        return ss.str();
    }

    static std::string boolean(bool b) {
        return b ? "true" : "false";
    }

    static std::string vec2(const sf::Vector2f& v) {
        return object({
            {"x", number(v.x)},
            {"y", number(v.y)}
            });
    }

    static std::string color(const sf::Color& c) {
        return object({
            {"r", number(static_cast<float>(c.r))},
            {"g", number(static_cast<float>(c.g))},
            {"b", number(static_cast<float>(c.b))},
            {"a", number(static_cast<float>(c.a))}
            });
    }

    static sf::Vector2f parseVec2(const std::string& json) {
        sf::Vector2f result{ 0.f, 0.f };
        size_t xPos = json.find("\"x\":");
        size_t yPos = json.find("\"y\":");
        if (xPos != std::string::npos) {
            result.x = std::stof(json.substr(xPos + 4));
        }
        if (yPos != std::string::npos) {
            result.y = std::stof(json.substr(yPos + 4));
        }
        return result;
    }

    static sf::Color parseColor(const std::string& json) {
        sf::Color result{ 255, 255, 255, 255 };
        size_t rPos = json.find("\"r\":");
        size_t gPos = json.find("\"g\":");
        size_t bPos = json.find("\"b\":");
        size_t aPos = json.find("\"a\":");
        if (rPos != std::string::npos) result.r = static_cast<uint8_t>(std::stof(json.substr(rPos + 4)));
        if (gPos != std::string::npos) result.g = static_cast<uint8_t>(std::stof(json.substr(gPos + 4)));
        if (bPos != std::string::npos) result.b = static_cast<uint8_t>(std::stof(json.substr(bPos + 4)));
        if (aPos != std::string::npos) result.a = static_cast<uint8_t>(std::stof(json.substr(aPos + 4)));
        return result;
    }

private:
    static std::string escape(const std::string& str) {
        std::string result;
        for (char c : str) {
            if (c == '"') result += "\\\"";
            else if (c == '\\') result += "\\\\";
            else result += c;
        }
        return result;
    }
};

// SceneSerializer implementation

SceneSerializer::SceneSerializer(Scene& scene)
    : m_Scene(scene) {
}

void SceneSerializer::newScene() {
    std::vector<Entity> toDestroy;
    for (auto& [entity, _] : m_Scene.transforms) {
        toDestroy.push_back(entity);
    }
    for (Entity entity : toDestroy) {
        m_Scene.destroyEntity(entity);
    }
}

bool SceneSerializer::save(const std::string& filepath) {
    fs::path path(filepath);
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path());
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }

    file << "{\n";
    file << "  \"version\": \"1.0\",\n";
    file << "  \"sceneName\": \"" << m_Scene.sceneName << "\",\n";
    file << "  \"entities\": [\n";

    bool first = true;
    for (auto& [entity, transform] : m_Scene.transforms) {
        if (!first) file << ",\n";
        first = false;

        file << "    {\n";
        file << "      \"id\": " << entity << ",\n";

        // Имя сущности
        if (m_Scene.names.find(entity) != m_Scene.names.end()) {
            file << "      \"name\": " << SimpleJson::string(m_Scene.names[entity].Name) << ",\n";
        }

        // Родитель
        if (m_Scene.parents.find(entity) != m_Scene.parents.end()) {
            file << "      \"parent\": " << m_Scene.parents[entity].Parent << ",\n";
        }

        file << "      \"transform\": " << serializeTransform(entity);

        if (m_Scene.sprites.find(entity) != m_Scene.sprites.end()) {
            file << ",\n      \"sprite\": " << serializeSprite(entity);
        }
        if (m_Scene.velocities.find(entity) != m_Scene.velocities.end()) {
            file << ",\n      \"velocity\": " << serializeVelocity(entity);
        }
        if (m_Scene.colliders.find(entity) != m_Scene.colliders.end()) {
            file << ",\n      \"collider\": " << serializeCollider(entity);
        }
        if (m_Scene.healths.find(entity) != m_Scene.healths.end()) {
            file << ",\n      \"health\": " << serializeHealth(entity);
        }
        if (m_Scene.tags.find(entity) != m_Scene.tags.end()) {
            file << ",\n      \"tag\": " << serializeTag(entity);
        }

        file << "\n    }";
    }

    file << "\n  ]\n}\n";
    file.close();
    return true;
}

bool SceneSerializer::load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }

    newScene();

    std::string line, json;
    while (std::getline(file, line)) json += line;
    file.close();

    // Загружаем имя сцены
    size_t sceneNamePos = json.find("\"sceneName\":");
    if (sceneNamePos != std::string::npos) {
        size_t start = json.find("\"", sceneNamePos + 13) + 1;
        size_t end = json.find("\"", start);
        if (start != std::string::npos && end != std::string::npos) {
            m_Scene.sceneName = json.substr(start, end - start);
            std::cout << "Loaded scene: " << m_Scene.sceneName << std::endl;
        }
    }

    // Карты для маппинга старых ID на новые
    std::map<Entity, Entity> oldToNewIds;
    std::map<Entity, Entity> pendingParents;

    // Загружаем сущности
    size_t pos = 0;
    while ((pos = json.find("\"id\":", pos)) != std::string::npos) {
        size_t idStart = json.find_first_of("0123456789", pos);
        size_t idEnd = json.find_first_not_of("0123456789", idStart);

        Entity oldId = static_cast<Entity>(std::stoi(json.substr(idStart, idEnd - idStart)));
        Entity newEntity = m_Scene.createEntity();
        oldToNewIds[oldId] = newEntity;

        if (newEntity == INVALID_ENTITY) continue;

        size_t objStart = json.rfind("{", pos);
        size_t objEnd = objStart;
        int braceCount = 0;
        for (size_t i = objStart; i < json.length(); ++i) {
            if (json[i] == '{') braceCount++;
            if (json[i] == '}') {
                braceCount--;
                if (braceCount == 0) { objEnd = i; break; }
            }
        }

        std::string entityData = json.substr(objStart, objEnd - objStart + 1);

        // Имя сущности
        size_t namePos = entityData.find("\"name\":");
        if (namePos != std::string::npos) {
            size_t start = entityData.find("\"", namePos + 7) + 1;
            size_t end = entityData.find("\"", start);
            if (start != std::string::npos && end != std::string::npos) {
                m_Scene.setEntityName(newEntity, entityData.substr(start, end - start));
            }
        }

        // Родитель (отложенная установка)
        size_t parentPos = entityData.find("\"parent\":");
        if (parentPos != std::string::npos) {
            size_t numStart = entityData.find_first_of("0123456789", parentPos);
            if (numStart != std::string::npos) {
                Entity parentId = static_cast<Entity>(std::stoi(entityData.substr(numStart)));
                pendingParents[newEntity] = parentId;
            }
        }

        deserializeTransform(newEntity, entityData);

        if (entityData.find("\"sprite\":") != std::string::npos) {
            deserializeSprite(newEntity, entityData);
        }
        if (entityData.find("\"velocity\":") != std::string::npos) {
            deserializeVelocity(newEntity, entityData);
        }
        if (entityData.find("\"collider\":") != std::string::npos) {
            deserializeCollider(newEntity, entityData);
        }
        if (entityData.find("\"health\":") != std::string::npos) {
            deserializeHealth(newEntity, entityData);
        }
        if (entityData.find("\"tag\":") != std::string::npos) {
            deserializeTag(newEntity, entityData);
        }

        pos = objEnd + 1;
    }

    // Устанавливаем родителей после создания всех сущностей
    for (const auto& pair : pendingParents) {
        Entity childId = pair.first;
        Entity parentOldId = pair.second;

        auto it = oldToNewIds.find(parentOldId);
        if (it != oldToNewIds.end()) {
            Entity newParent = it->second;
            m_Scene.setParent(childId, newParent);
        }
    }

    return true;
}

std::string SceneSerializer::serializeTransform(Entity entity) {
    auto& tf = m_Scene.transforms[entity];
    return SimpleJson::object({
        {"position", SimpleJson::vec2(tf.Pos)},
        {"rotation", SimpleJson::number(tf.Rot.asDegrees())},
        {"scale", SimpleJson::vec2(tf.Scale)}
        });
}

std::string SceneSerializer::serializeSprite(Entity entity) {
    auto& sprite = m_Scene.sprites[entity];
    std::string texPath = sprite.texturePath.empty() ? "null" : sprite.texturePath;
    return SimpleJson::object({
        {"texturePath", SimpleJson::string(texPath)},
        {"layer", SimpleJson::number(static_cast<float>(sprite.layer))},
        {"visible", SimpleJson::boolean(sprite.visible)}
        });
}

std::string SceneSerializer::serializeVelocity(Entity entity) {
    auto& vel = m_Scene.velocities[entity];
    return SimpleJson::object({
        {"velocity", SimpleJson::vec2(vel.Velocity)},
        {"maxSpeed", SimpleJson::number(vel.maxSpeed)},
        {"damping", SimpleJson::number(vel.damping)}
        });
}

std::string SceneSerializer::serializeCollider(Entity entity) {
    auto& col = m_Scene.colliders[entity];
    return SimpleJson::object({
        {"shape", SimpleJson::number(col.shape == ColliderComponent::Shape::Box ? 0.f : 1.f)},
        {"size", SimpleJson::vec2(col.size)},
        {"radius", SimpleJson::number(col.radius)},
        {"isTrigger", SimpleJson::boolean(col.isTrigger)},
        {"isStatic", SimpleJson::boolean(col.isStatic)}
        });
}

std::string SceneSerializer::serializeHealth(Entity entity) {
    auto& health = m_Scene.healths[entity];
    return SimpleJson::object({
        {"current", SimpleJson::number(static_cast<float>(health.currentHealth))},
        {"max", SimpleJson::number(static_cast<float>(health.maxHealth))}
        });
}

std::string SceneSerializer::serializeTag(Entity entity) {
    auto& tag = m_Scene.tags[entity];
    return SimpleJson::object({
        {"name", SimpleJson::string(tag.Tag)}
        });
}

void SceneSerializer::deserializeTransform(Entity entity, const std::string& json) {
    auto& tf = m_Scene.transforms[entity];

    size_t posPos = json.find("\"position\":");
    if (posPos != std::string::npos) {
        size_t start = json.find("{", posPos);
        size_t end = json.find("}", start);
        if (start != std::string::npos && end != std::string::npos) {
            tf.Pos = SimpleJson::parseVec2(json.substr(start, end - start + 1));
        }
    }

    size_t scalePos = json.find("\"scale\":");
    if (scalePos != std::string::npos) {
        size_t start = json.find("{", scalePos);
        size_t end = json.find("}", start);
        if (start != std::string::npos && end != std::string::npos) {
            tf.Scale = SimpleJson::parseVec2(json.substr(start, end - start + 1));
        }
    }

    size_t rotPos = json.find("\"rotation\":");
    if (rotPos != std::string::npos) {
        size_t numStart = json.find_first_of("-0123456789", rotPos);
        if (numStart != std::string::npos) {
            tf.Rot = sf::degrees(std::stof(json.substr(numStart)));
        }
    }
}

void SceneSerializer::deserializeSprite(Entity entity, const std::string& json) {
    m_Scene.sprites[entity] = SpriteComponent{};
    auto& sprite = m_Scene.sprites[entity];

    size_t texPos = json.find("\"texturePath\":");
    if (texPos != std::string::npos) {
        size_t start = json.find("\"", texPos + 14) + 1;
        size_t end = json.find("\"", start);
        if (start != std::string::npos && end != std::string::npos) {
            std::string path = json.substr(start, end - start);
            sprite.texturePath = path;
            if (path != "null" && fs::exists(path)) {
                try {
                    auto tex = ResourceManager::loadTexture(path);
                    m_Scene.setTexture(entity, tex, path);
                }
                catch (...) {}
            }
        }
    }

    size_t layerPos = json.find("\"layer\":");
    if (layerPos != std::string::npos) {
        size_t numStart = json.find_first_of("-0123456789", layerPos);
        if (numStart != std::string::npos) {
            sprite.layer = static_cast<int>(std::stof(json.substr(numStart)));
        }
    }
}

void SceneSerializer::deserializeVelocity(Entity entity, const std::string& json) {
    m_Scene.velocities[entity] = VelocityComponent{};
    auto& vel = m_Scene.velocities[entity];

    size_t velPos = json.find("\"velocity\":");
    if (velPos != std::string::npos) {
        size_t start = json.find("{", velPos);
        size_t end = json.find("}", start);
        if (start != std::string::npos && end != std::string::npos) {
            vel.Velocity = SimpleJson::parseVec2(json.substr(start, end - start + 1));
        }
    }
}

void SceneSerializer::deserializeCollider(Entity entity, const std::string& json) {
    m_Scene.colliders[entity] = ColliderComponent{};
}

void SceneSerializer::deserializeHealth(Entity entity, const std::string& json) {
    m_Scene.healths[entity] = HealthComponent{};
}

void SceneSerializer::deserializeTag(Entity entity, const std::string& json) {
    m_Scene.tags[entity] = TagComponent{};

    size_t namePos = json.find("\"name\":");
    if (namePos != std::string::npos) {
        size_t start = json.find("\"", namePos + 7) + 1;
        size_t end = json.find("\"", start);
        if (start != std::string::npos && end != std::string::npos) {
            m_Scene.tags[entity].Tag = json.substr(start, end - start);
        }
    }
}