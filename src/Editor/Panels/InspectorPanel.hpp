#pragma once

#include <imgui.h>
#include "../../ECS/Scene.hpp"
#include <string>
#include <functional>

class EditorUI; // Forward declaration

class InspectorPanel {
public:
    InspectorPanel(Scene& scene);
    ~InspectorPanel() = default;

    void render(Entity entity);

    void setOnAddSprite(std::function<void(Entity)> callback) { m_OnAddSprite = callback; }
    void setOnOpenScriptEditor(std::function<void(Entity)> callback) { m_OnOpenScriptEditor = callback; }

private:
    Scene& m_Scene;
    char m_EntityNameBuffer[64] = "";

    std::function<void(Entity)> m_OnAddSprite;
    std::function<void(Entity)> m_OnOpenScriptEditor;

    void renderTransform(Entity entity);
    void renderSprite(Entity entity);
    void renderVelocity(Entity entity);
    void renderAddComponentMenu(Entity entity);
};