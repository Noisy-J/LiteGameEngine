#include "TextureSelectorDialog.hpp"
#include "../../Resources/TextureBrowser.hpp"
#include <filesystem>
#include <iostream>

TextureSelectorDialog::TextureSelectorDialog(Scene& scene)
    : m_Scene(scene) {
}

void TextureSelectorDialog::open(Entity entity) {
    std::cout << "TextureSelectorDialog::open called for entity: " << entity << std::endl;
    m_IsOpen = true;
    m_TargetEntity = entity;
    m_ShouldOpenPopup = true;  // Добавь флаг
    scanTextures();
}

void TextureSelectorDialog::render() {
    if (!m_IsOpen) return;

    // Открываем попап если нужно
    if (m_ShouldOpenPopup) {
        ImGui::OpenPopup("Select Texture");
        m_ShouldOpenPopup = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Select Texture", &m_IsOpen,
        ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::Text("Choose texture:");
        ImGui::Separator();

        // Комбобокс с доступными текстурами
        if (ImGui::BeginCombo("Texture",
            m_AvailableTextures.empty() ? "" :
            std::filesystem::path(m_AvailableTextures[m_SelectedTextureIndex])
            .filename().string().c_str())) {

            for (int i = 0; i < m_AvailableTextures.size(); i++) {
                bool isSelected = (m_SelectedTextureIndex == i);
                std::string filename = std::filesystem::path(m_AvailableTextures[i])
                    .filename().string();

                if (ImGui::Selectable(filename.c_str(), isSelected)) {
                    m_SelectedTextureIndex = i;
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::InputText("Or enter path", m_TexturePathBuffer,
            sizeof(m_TexturePathBuffer));

        ImGui::Separator();

        if (ImGui::Button("Apply", ImVec2(120, 0))) {
            applyTexture();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            cancel();
        }

        ImGui::EndPopup();
    }
}

void TextureSelectorDialog::scanTextures() {
    TextureBrowser browser(m_Scene);
    browser.scanAvailableTextures();
    m_AvailableTextures = browser.getAvailableTextures();
}

void TextureSelectorDialog::applyTexture() {
    // Сначала проверяем, есть ли у сущности спрайт компонент
    if (!m_Scene.sprites.contains(m_TargetEntity)) {
        // Если нет - создаём пустой
        m_Scene.sprites[m_TargetEntity] = SpriteComponent{};
    }

    // Загружаем текстуру
    if (!m_AvailableTextures.empty() && m_SelectedTextureIndex < m_AvailableTextures.size()) {
        TextureBrowser browser(m_Scene);
        browser.loadTextureToEntity(m_TargetEntity,
            m_AvailableTextures[m_SelectedTextureIndex]);
    }
    else if (strlen(m_TexturePathBuffer) > 0) {
        TextureBrowser browser(m_Scene);
        browser.loadTextureToEntity(m_TargetEntity, m_TexturePathBuffer);
    }

    m_IsOpen = false;
    m_TexturePathBuffer[0] = '\0';
    ImGui::CloseCurrentPopup();
}

void TextureSelectorDialog::cancel() {
    m_IsOpen = false;
    m_TexturePathBuffer[0] = '\0';
    ImGui::CloseCurrentPopup();
}

