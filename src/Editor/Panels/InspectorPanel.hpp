#pragma once

#include <imgui.h>
#include "../../ECS/Scene.hpp"
#include <string>
#include <functional>
#include <iostream>

class InspectorPanel {
public:
    InspectorPanel(Scene& scene);
    ~InspectorPanel() = default;

    void render(Entity entity);

    // Callback для открытия диалога текстур
    void setOnAddSprite(std::function<void(Entity)> callback) { m_OnAddSprite = callback; }

private:
    Scene& m_Scene;
    char m_EntityNameBuffer[64] = "";

    std::function<void(Entity)> m_OnAddSprite;

    void renderTransform(Entity entity);
    void renderSprite(Entity entity);
    void renderVelocity(Entity entity);
    void renderAddComponentMenu(Entity entity);
};