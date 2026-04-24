#pragma once

#include <imgui.h>
#include "../../Rendering/Viewport.hpp"

class ViewportPanel {
public:
    ViewportPanel(Viewport& viewport);
    ~ViewportPanel() = default;
    

    void render();

private:
    Viewport& m_Viewport;

    // Состояние тулбара
    bool m_ShowGridOverlay{ true };
    bool m_ShowBounds{ true };
    bool m_ShowColliders{ false };
    int m_MouseMode{ 0 }; // 0: Select, 1: Pan, 2: Zoom
    bool m_ShowDebugPopup{ false };

    void renderToolbar();
    void handleResize();
    void renderOverlaySettings();
};