#pragma once

#include <vector>
#include <string>
#include "../ECS/Scene.hpp"

class TextureBrowser {
public:
    TextureBrowser(Scene& scene);
    ~TextureBrowser() = default;

    void scanAvailableTextures(const std::string& directory = "./assets");
    void loadTextureToEntity(Entity entity, const std::string& path);

    const std::vector<std::string>& getAvailableTextures() const { return m_AvailableTextures; }

private:
    Scene& m_Scene;
    std::vector<std::string> m_AvailableTextures;
};