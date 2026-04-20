#pragma once

#include <imgui.h>
#include <vector>
#include <string>

class ContentBrowserPanel {
public:
    ContentBrowserPanel();
    ~ContentBrowserPanel() = default;

    void render();

private:
    std::string m_CurrentDirectory{ "./assets" };
    std::vector<std::string> m_Items;
    float m_ThumbnailSize{ 64.f };

    void refreshDirectory();
    void renderNavigationBar();
    void renderFileGrid();
};