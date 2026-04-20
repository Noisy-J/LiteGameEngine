#pragma once

#include <imgui.h>
#include "../../ECS/Scene.hpp"
#include <string>

class InspectorPanel {
public:
    InspectorPanel(Scene& scene);
    ~InspectorPanel() = default;

    void render(Entity entity);

private:
    Scene& m_Scene;
    char m_EntityNameBuffer[64] = "";

    void renderTransform(Entity entity);
    void renderSprite(Entity entity);
    void renderVelocity(Entity entity);
    void renderAddComponentMenu(Entity entity);

    // Вспомогательные функции
    void beginPropertyChange(const std::string& propertyName);
    void endPropertyChange();
};