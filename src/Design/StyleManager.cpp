#include "StyleManager.hpp"
#include <imgui-SFML.h>
#include <algorithm>

StyleManager::StyleManager() {
    // Регистрируем стили
    registerStyle("Default", []() { applyDefaultStyle(); });
    registerStyle("Alternative", []() { applyAlternativeStyle(); });
    registerStyle("Dark", []() { applyDarkStyle(); });
    registerStyle("Light", []() { applyLightStyle(); });
    registerStyle("Cyberpunk", []() { applyCyberpunkStyle(); });
    registerStyle("Modern Dark", []() { applyModernDarkStyle(); });

    m_CurrentStyleName = "Modern Dark";
    m_CurrentPreset = Preset::ModernDark;
}

void StyleManager::registerStyle(const std::string& name, std::function<void()> styleFunc) {
    m_CustomStyles[name] = styleFunc;
}

void StyleManager::applyStyle(Preset preset) {
    switch (preset) {
    case Preset::Default:      applyDefaultStyle(); m_CurrentStyleName = "Default"; break;
    case Preset::Alternative:  applyAlternativeStyle(); m_CurrentStyleName = "Alternative"; break;
    case Preset::Dark:         applyDarkStyle(); m_CurrentStyleName = "Dark"; break;
    case Preset::Light:        applyLightStyle(); m_CurrentStyleName = "Light"; break;
    case Preset::Cyberpunk:    applyCyberpunkStyle(); m_CurrentStyleName = "Cyberpunk"; break;
    case Preset::ModernDark:   applyModernDarkStyle(); m_CurrentStyleName = "Modern Dark"; break;
    default: break;
    }
    m_CurrentPreset = preset;
}

void StyleManager::applyStyle(const std::string& name) {
    auto it = m_CustomStyles.find(name);
    if (it != m_CustomStyles.end()) {
        it->second();
        m_CurrentStyleName = name;
        m_CurrentPreset = Preset::Custom;
    }
}

std::vector<std::string> StyleManager::getAvailableStyles() const {
    std::vector<std::string> styles;
    for (const auto& [name, _] : m_CustomStyles) {
        styles.push_back(name);
    }
    std::sort(styles.begin(), styles.end());
    return styles;
}

void StyleManager::renderStyleSelector() {
    if (ImGui::BeginMenu("Style")) {
        auto styles = getAvailableStyles();
        for (const auto& styleName : styles) {
            bool isSelected = (m_CurrentStyleName == styleName);
            if (ImGui::MenuItem(styleName.c_str(), nullptr, isSelected)) {
                applyStyle(styleName);
            }
        }
        ImGui::EndMenu();
    }
}

// === Реализации стилей ===

void StyleManager::applyDefaultStyle() {
    ImGui::StyleColorsDark();
}

void StyleManager::applyAlternativeStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowBorderSize = 1.0f;

    const ImVec4 Black = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
    const ImVec4 DarkGray = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    const ImVec4 NeonPink = ImVec4(1.00f, 0.00f, 0.65f, 1.00f);
    const ImVec4 ToxicGreen = ImVec4(0.00f, 1.00f, 0.40f, 1.00f);
    const ImVec4 BgAlpha = ImVec4(0.05f, 0.05f, 0.05f, 0.94f);

    colors[ImGuiCol_WindowBg] = BgAlpha;
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_Border] = NeonPink;
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    colors[ImGuiCol_TitleBg] = DarkGray;
    colors[ImGuiCol_TitleBgActive] = DarkGray;
    colors[ImGuiCol_TitleBgCollapsed] = DarkGray;

    colors[ImGuiCol_Text] = ToxicGreen;
    colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);

    colors[ImGuiCol_FrameBg] = Black;
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = NeonPink;

    colors[ImGuiCol_Button] = Black;
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ToxicGreen;

    colors[ImGuiCol_ScrollbarBg] = Black;
    colors[ImGuiCol_ScrollbarGrab] = NeonPink;
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.00f, 0.20f, 0.75f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ToxicGreen;

    colors[ImGuiCol_SliderGrab] = NeonPink;
    colors[ImGuiCol_SliderGrabActive] = ToxicGreen;

    colors[ImGuiCol_Tab] = Black;
    colors[ImGuiCol_TabHovered] = NeonPink;
    colors[ImGuiCol_TabActive] = NeonPink;
    colors[ImGuiCol_TabUnfocused] = Black;
    colors[ImGuiCol_TabUnfocusedActive] = DarkGray;

    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = NeonPink;
    colors[ImGuiCol_HeaderActive] = ToxicGreen;

    colors[ImGuiCol_CheckMark] = ToxicGreen;
    colors[ImGuiCol_Separator] = NeonPink;
    colors[ImGuiCol_ResizeGrip] = NeonPink;
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 0.20f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ToxicGreen;
    colors[ImGuiCol_PlotLines] = ToxicGreen;
    colors[ImGuiCol_PlotLinesHovered] = NeonPink;
    colors[ImGuiCol_TextSelectedBg] = ImVec4(NeonPink.x, NeonPink.y, NeonPink.z, 0.35f);
    colors[ImGuiCol_DragDropTarget] = NeonPink;
    colors[ImGuiCol_NavHighlight] = ToxicGreen;
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);
}

void StyleManager::applyDarkStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.FrameBorderSize = 0.0f;

    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.40f, 0.42f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.35f, 0.37f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.45f, 0.45f, 0.47f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
}

void StyleManager::applyLightStyle() {
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
}

void StyleManager::applyCyberpunkStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding = 2.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.FrameBorderSize = 1.0f;

    const ImVec4 CyberYellow = ImVec4(1.00f, 0.85f, 0.00f, 1.00f);
    const ImVec4 CyberBlue = ImVec4(0.00f, 0.80f, 1.00f, 1.00f);
    const ImVec4 DarkBg = ImVec4(0.02f, 0.05f, 0.10f, 1.00f);
    const ImVec4 PanelBg = ImVec4(0.05f, 0.10f, 0.15f, 1.00f);

    colors[ImGuiCol_WindowBg] = DarkBg;
    colors[ImGuiCol_Text] = CyberYellow;
    colors[ImGuiCol_Border] = CyberBlue;
    colors[ImGuiCol_Button] = PanelBg;
    colors[ImGuiCol_ButtonHovered] = CyberBlue;
    colors[ImGuiCol_ButtonActive] = CyberYellow;
    colors[ImGuiCol_TitleBg] = PanelBg;
    colors[ImGuiCol_TitleBgActive] = CyberBlue;
    colors[ImGuiCol_Header] = PanelBg;
    colors[ImGuiCol_HeaderHovered] = CyberBlue;
    colors[ImGuiCol_HeaderActive] = CyberYellow;
    colors[ImGuiCol_FrameBg] = PanelBg;
    colors[ImGuiCol_CheckMark] = CyberBlue;
    colors[ImGuiCol_SliderGrab] = CyberBlue;
}

void StyleManager::applyModernDarkStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding = 6.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;

    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

    const ImVec4 BgColor = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    const ImVec4 SecondaryBg = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    const ImVec4 HeaderColor = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    const ImVec4 AccentColor = ImVec4(0.15f, 0.42f, 0.65f, 1.00f);
    const ImVec4 AccentHover = ImVec4(0.19f, 0.53f, 0.82f, 1.00f);
    const ImVec4 TextMain = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    const ImVec4 TextDim = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);

    colors[ImGuiCol_WindowBg] = BgColor;
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = SecondaryBg;
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);

    colors[ImGuiCol_TitleBg] = HeaderColor;
    colors[ImGuiCol_TitleBgActive] = HeaderColor;
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);

    colors[ImGuiCol_Text] = TextMain;
    colors[ImGuiCol_TextDisabled] = TextDim;

    colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);

    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive] = AccentColor;

    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);

    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered] = AccentColor;
    colors[ImGuiCol_TabActive] = AccentColor;
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = SecondaryBg;

    colors[ImGuiCol_Header] = AccentColor;
    colors[ImGuiCol_HeaderHovered] = AccentHover;
    colors[ImGuiCol_HeaderActive] = AccentHover;

    colors[ImGuiCol_CheckMark] = AccentColor;
    colors[ImGuiCol_SliderGrab] = AccentColor;
    colors[ImGuiCol_SliderGrabActive] = AccentHover;

    colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = AccentColor;
    colors[ImGuiCol_ResizeGripHovered] = AccentHover;
    colors[ImGuiCol_ResizeGripActive] = AccentHover;
    colors[ImGuiCol_PlotLines] = AccentColor;
    colors[ImGuiCol_PlotLinesHovered] = AccentHover;
    colors[ImGuiCol_TextSelectedBg] = ImVec4(AccentColor.x, AccentColor.y, AccentColor.z, 0.35f);
    colors[ImGuiCol_DragDropTarget] = AccentColor;
    colors[ImGuiCol_NavHighlight] = AccentColor;
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);
}