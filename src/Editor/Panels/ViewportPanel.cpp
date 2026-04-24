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

    m_Viewport.setScreenSize({ viewportSize.x, viewportSize.y });
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    m_Viewport.setScreenPosition({ cursorPos.x, cursorPos.y });

    handleResize();

    sf::Vector2f size(static_cast<float>(m_Viewport.getSize().x),
        static_cast<float>(m_Viewport.getSize().y));
    ImGui::Image(m_Viewport.getTexture(), size);

    // Отслеживаем вход/выход мыши из Viewport
    if (ImGui::IsItemHovered()) {
        if (!m_Viewport.isMouseInside()) {
            // Мышь только что вошла
            m_Viewport.setMouseInside(true);
        }
        // Меняем курсор в зависимости от режима
        switch (m_Viewport.getMouseMode()) {
        case Viewport::MouseMode::Select:
            ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
            break;
        case Viewport::MouseMode::Pan:
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            break;
        case Viewport::MouseMode::Zoom:
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
            break;
        }
    }
    else {
        if (m_Viewport.isMouseInside()) {
            // Мышь только что вышла
            m_Viewport.setMouseInside(false);
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void ViewportPanel::renderToolbar() {
    ImGui::BeginChild("ViewportToolbar", ImVec2(0, 25));

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 0));

    // Кнопка Select
    bool isSelect = (m_Viewport.getMouseMode() == Viewport::MouseMode::Select);
    if (isSelect) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.7f, 0.8f));
    if (ImGui::Button("Select", ImVec2(50, 20))) {
        m_Viewport.setMouseMode(Viewport::MouseMode::Select);
    }
    if (isSelect) ImGui::PopStyleColor();

    ImGui::SameLine();

    // Кнопка Pan
    bool isPan = (m_Viewport.getMouseMode() == Viewport::MouseMode::Pan);
    if (isPan) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.7f, 0.8f));
    if (ImGui::Button("Pan", ImVec2(40, 20))) {
        m_Viewport.setMouseMode(Viewport::MouseMode::Pan);
    }
    if (isPan) ImGui::PopStyleColor();

    ImGui::SameLine();

    // Кнопка Zoom
    bool isZoom = (m_Viewport.getMouseMode() == Viewport::MouseMode::Zoom);
    if (isZoom) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.7f, 0.8f));
    if (ImGui::Button("Zoom", ImVec2(40, 20))) {
        m_Viewport.setMouseMode(Viewport::MouseMode::Zoom);
    }
    if (isZoom) ImGui::PopStyleColor();

    ImGui::PopStyleVar();

    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.0f), "|");
    ImGui::SameLine();

    // Кнопка Debug - открываем попап
    if (ImGui::Button("Debug")) {
        m_ShowDebugPopup = true;
    }

    // Текущий зум
    ImGui::SameLine();
    sf::View view = m_Viewport.getView();
    float zoom = std::abs(view.getSize().x) / static_cast<float>(m_Viewport.getSize().x);
    ImGui::Text("Zoom: %.2fx", 1.0f / zoom);

    ImGui::EndChild();

    // Рендерим попап здесь, после тулбара но до конца окна Viewport
    if (m_ShowDebugPopup) {
        ImGui::OpenPopup("ViewportSettings");
        m_ShowDebugPopup = false;
    }

    // Всегда проверяем попап
    if (ImGui::BeginPopup("ViewportSettings")) {
        ImGui::Text("Viewport Display Settings");
        ImGui::Separator();

        bool gridChanged = ImGui::Checkbox("Show Grid", &m_ShowGridOverlay);
        bool boundsChanged = ImGui::Checkbox("Show Bounds", &m_ShowBounds);
        bool collidersChanged = ImGui::Checkbox("Show Colliders", &m_ShowColliders);

        if (gridChanged) {
            m_Viewport.setShowGrid(m_ShowGridOverlay);
        }
        if (boundsChanged) {
            m_Viewport.setShowBounds(m_ShowBounds);
        }
        if (collidersChanged) {
            m_Viewport.setShowColliders(m_ShowColliders);
        }

        ImGui::Separator();
        ImGui::Text("Grid Settings");

        float gridSize = m_Viewport.getGridSize();
        if (ImGui::DragFloat("Grid Size", &gridSize, 10.f, 10.f, 500.f)) {
            m_Viewport.setGridSize(gridSize);
        }

        ImGui::Separator();
        ImGui::Text("Mouse Mode");
        ImGui::RadioButton("Select", &m_MouseMode, 0);
        ImGui::RadioButton("Pan", &m_MouseMode, 1);
        ImGui::RadioButton("Zoom", &m_MouseMode, 2);

        if (m_MouseMode == 0) m_Viewport.setMouseMode(Viewport::MouseMode::Select);
        else if (m_MouseMode == 1) m_Viewport.setMouseMode(Viewport::MouseMode::Pan);
        else m_Viewport.setMouseMode(Viewport::MouseMode::Zoom);

        ImGui::EndPopup();
    }
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