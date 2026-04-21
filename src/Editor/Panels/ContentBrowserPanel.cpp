#include "ContentBrowserPanel.hpp"
#include <filesystem>

ContentBrowserPanel::ContentBrowserPanel() {
    refreshDirectory();
}

void ContentBrowserPanel::render() {
    ImGui::Begin("Content Browser");

    renderNavigationBar();

    ImGui::Separator();

    renderFileGrid();

    ImGui::End();
}

void ContentBrowserPanel::refreshDirectory() {
    m_Items.clear();

    try {
        for (const auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
            m_Items.push_back(entry.path().filename().string());
        }
    }
    catch (...) {
        // Обработка ошибок
    }
}

void ContentBrowserPanel::renderNavigationBar() {
    if (ImGui::Button("<")) {
        // Назад
    }
    ImGui::SameLine();
    if (ImGui::Button(">")) {
        // Вперёд
    }
    ImGui::SameLine();
    if (ImGui::Button("^")) {
        auto parent = std::filesystem::path(m_CurrentDirectory).parent_path();
        if (!parent.empty()) {
            m_CurrentDirectory = parent.string();
            refreshDirectory();
        }
    }
    ImGui::SameLine();

    ImGui::Text("%s", m_CurrentDirectory.c_str());

    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    ImGui::SliderFloat("Size", &m_ThumbnailSize, 32.f, 128.f, "%.0f");
}

void ContentBrowserPanel::renderFileGrid() {
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columns = std::max(1, static_cast<int>(panelWidth / (m_ThumbnailSize + 16.f)));

    ImGui::Columns(columns, nullptr, false);

    for (const auto& item : m_Items) {
        ImGui::PushID(item.c_str());

        // Кнопка-превью
        if (ImGui::Button(item.c_str(), ImVec2(m_ThumbnailSize, m_ThumbnailSize))) {
            //Реализация деректории
        }

        // Drag and drop
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("ASSET_PATH", item.c_str(), item.size() + 1);
            ImGui::Text("Dragging %s", item.c_str());
            ImGui::EndDragDropSource();
        }

        // Название файла
        ImGui::TextWrapped("%s", item.c_str());

        ImGui::PopID();
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
}