#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include "../../ECS/Scene.hpp"

class TextureSelectorDialog {
public:
    TextureSelectorDialog(Scene& scene);
    ~TextureSelectorDialog() = default;

    void open(Entity entity);
    void render();
    bool isOpen() const { return m_IsOpen; }

private:
    Scene& m_Scene;
    bool m_IsOpen{ false };
    Entity m_TargetEntity{ INVALID_ENTITY };

    std::vector<std::string> m_AvailableTextures;
    int m_SelectedTextureIndex{ 0 };
    char m_TexturePathBuffer[256] = "";

    void scanTextures();
    void applyTexture();
    void cancel();
};