#include "ViewportPanel.hpp"
#include <imgui-SFML.h>

ViewportPanel::ViewportPanel(Viewport& viewport)
    : m_Viewport(viewport) {
}

void ViewportPanel::render() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    renderToolbar();

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    // Обновляем размер и позицию вьюпорта
    m_Viewport.setScreenSize({ viewportSize.x, viewportSize.y });

    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    m_Viewport.setScreenPosition({ cursorPos.x, cursorPos.y });

    handleResize();

    // Правильный способ для ImGui-SFML
    sf::Vector2f size(static_cast<float>(m_Viewport.getSize().x),
        static_cast<float>(m_Viewport.getSize().y));
    ImGui::Image(m_Viewport.getTexture(), size);

    ImGui::End();
    ImGui::PopStyleVar();
}

void ViewportPanel::renderToolbar() {
    ImGui::BeginChild("ViewportToolbar", ImVec2(0, 20));

    if (ImGui::Button("Mode")) {

    }
    ImGui::SameLine(0, 1);
    if (ImGui::Button("View")) {

    }
    //ImGui::SameLine();
    //if (ImGui::Button("Rotate")) {}
    //ImGui::SameLine();
    //if (ImGui::Button("Scale")) {}

    ImGui::EndChild();
}

void ViewportPanel::handleResize() {
    auto currentSize = m_Viewport.getSize();
    auto screenSize = m_Viewport.getScreenSize();

    if (screenSize.x > 0 && screenSize.y > 0) {
        if (static_cast<unsigned int>(screenSize.x) != currentSize.x ||
            static_cast<unsigned int>(screenSize.y) != currentSize.y) {

            sf::View oldView = m_Viewport.getView();
            sf::Vector2f oldSize = oldView.getSize();

            float zoomX = oldSize.x / static_cast<float>(currentSize.x);
            float zoomY = oldSize.y / static_cast<float>(currentSize.y);

            m_Viewport.resize({ static_cast<unsigned int>(screenSize.x),
                              static_cast<unsigned int>(screenSize.y) });

            sf::View newView = m_Viewport.getView();
            newView.setSize({ screenSize.x * zoomX,
                            -screenSize.y * std::abs(zoomY) });
            m_Viewport.setView(newView);
        }
    }
}