#pragma once

#include <imgui.h>
#include <imgui-SFML.h>
#include "../../Rendering/Viewport.hpp"

class ViewportPanel {
public:
    ViewportPanel(Viewport& viewport);
    ~ViewportPanel() = default;

    void render();

private:
    Viewport& m_Viewport;

    void handleResize();
    void renderToolbar();
};