#pragma once

#include <imgui.h>
#include "../../ECS/Scene.hpp"

class DebugPanel {
public:
    DebugPanel() = default;
    ~DebugPanel() = default;

    void render(Scene& scene, Entity selectedEntity);

private:
    bool m_ShowBounds{ true };
    bool m_ShowColliders{ true };
    bool m_ShowVelocity{ false };
};