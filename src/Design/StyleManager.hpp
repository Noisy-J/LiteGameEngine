#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

class StyleManager {
public:
    enum class Preset {
        Default,
        Alternative,
        Dark,
        Light,
        Cyberpunk,
        ModernDark,  // Добавлен
        Custom
    };

    static StyleManager& getInstance() {
        static StyleManager instance;
        return instance;
    }

    void applyStyle(Preset preset);
    void applyStyle(const std::string& name);

    void registerStyle(const std::string& name, std::function<void()> styleFunc);

    std::vector<std::string> getAvailableStyles() const;

    const std::string& getCurrentStyleName() const { return m_CurrentStyleName; }
    Preset getCurrentPreset() const { return m_CurrentPreset; }

    void renderStyleSelector();

    // Пресеты
    static void applyDefaultStyle();
    static void applyAlternativeStyle();
    static void applyDarkStyle();
    static void applyLightStyle();
    static void applyCyberpunkStyle();
    static void applyModernDarkStyle();  // Добавлен

private:
    StyleManager();
    ~StyleManager() = default;

    std::string m_CurrentStyleName = "ModernDark";
    Preset m_CurrentPreset = Preset::ModernDark;

    std::unordered_map<std::string, std::function<void()>> m_CustomStyles;
};