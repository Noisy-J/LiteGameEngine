#include "CreateEntityDialog.hpp"
#include "Resources/TextureBrowser.hpp"
#include "Resources/ResourceManager.hpp"
#include <filesystem>

CreateEntityDialog::CreateEntityDialog(Scene& scene)
    : m_Scene(scene) {
}

void CreateEntityDialog::open(const sf::Vector2f& worldPosition) {
    m_IsOpen = true;
    m_WorldPosition = worldPosition;
    m_CurrentStep = 0;

    // Сброс настроек
    strcpy_s(m_EntityName, "New Entity");
    m_AddSprite = false;
    m_AddVelocity = false;
    m_AddCollider = false;
    m_AddHealth = false;
    m_AddTag = false;

    // Сброс настроек компонентов
    m_SpriteColor = sf::Color::White;
    m_SpriteLayer = 0;
    m_InitialVelocity = { 0.f, 0.f };
    m_MaxSpeed = 1000.f;
    m_Damping = 0.f;
    m_ColliderShape = ColliderComponent::Shape::Box;
    m_BoxSize = { 32.f, 32.f };
    m_CircleRadius = 16.f;
    m_ColliderOffset = { 0.f, 0.f };
    m_IsTrigger = false;
    m_IsStatic = false;
    m_MaxHealth = 100;
    m_CurrentHealth = 100;
    strcpy_s(m_TagName, "Untagged");

    scanTextures();

    ImGui::OpenPopup("Create Entity##Dialog");
}

void CreateEntityDialog::render() {
    if (!m_IsOpen) return;

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Create Entity##Dialog", &m_IsOpen,
        ImGuiWindowFlags_NoResize)) {

        // Заголовок с шагами
        ImGui::Text("Step %d/3: %s", m_CurrentStep + 1, getStepName(m_CurrentStep));
        ImGui::Separator();

        // Основной контент
        ImGui::BeginChild("Content", ImVec2(0, -50));

        switch (m_CurrentStep) {
        case 0: renderBasicInfo(); break;
        case 1: renderComponentSelection(); break;
        case 2: renderComponentSettings(); break;
        }

        ImGui::EndChild();

        ImGui::Separator();

        // Кнопки навигации
        if (m_CurrentStep > 0) {
            if (ImGui::Button("< Back", ImVec2(100, 0))) {
                m_CurrentStep--;
            }
            ImGui::SameLine();
        }

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 210);

        if (m_CurrentStep < 2) {
            if (ImGui::Button("Next >", ImVec2(100, 0)) && canProceed()) {
                m_CurrentStep++;
            }
        }
        else {
            if (ImGui::Button("Create", ImVec2(100, 0)) && canProceed()) {
                createEntity();
                m_IsOpen = false;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 0))) {
            m_IsOpen = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void CreateEntityDialog::renderBasicInfo() {
    ImGui::Text("Entity Name:");
    ImGui::InputText("##Name", m_EntityName, sizeof(m_EntityName));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("World Position:");
    ImGui::Text("X: %.2f, Y: %.2f", m_WorldPosition.x, m_WorldPosition.y);

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
        "Position is set by clicking in the viewport");
}

void CreateEntityDialog::renderComponentSelection() {
    ImGui::Text("Select components to add:");
    ImGui::Spacing();

    ImGui::Checkbox("Sprite (visual representation)", &m_AddSprite);
    ImGui::Checkbox("Velocity (movement)", &m_AddVelocity);
    ImGui::Checkbox("Collider (physics collision)", &m_AddCollider);
    ImGui::Checkbox("Health (hit points)", &m_AddHealth);
    ImGui::Checkbox("Tag (identifier)", &m_AddTag);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
        "Transform component is always added automatically");
}

void CreateEntityDialog::renderComponentSettings() {
    float contentHeight = 300;
    //ImGui::BeginChild("Settings", ImVec2(0, contentHeight), ImGuiBackendFlags_);
    ImGui::BeginChild("Settings", ImVec2(0, contentHeight));

    if (m_AddSprite) {
        if (ImGui::CollapsingHeader("Sprite Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            renderSpriteSettings();
        }
    }

    if (m_AddVelocity) {
        if (ImGui::CollapsingHeader("Velocity Settings")) {
            renderVelocitySettings();
        }
    }

    if (m_AddCollider) {
        if (ImGui::CollapsingHeader("Collider Settings")) {
            renderColliderSettings();
        }
    }

    if (m_AddHealth) {
        if (ImGui::CollapsingHeader("Health Settings")) {
            renderHealthSettings();
        }
    }

    if (m_AddTag) {
        if (ImGui::CollapsingHeader("Tag Settings")) {
            renderTagSettings();
        }
    }

    if (!m_AddSprite && !m_AddVelocity && !m_AddCollider && !m_AddHealth && !m_AddTag) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "No additional components selected");
        ImGui::Text("Only Transform component will be added");
    }

    ImGui::EndChild();
}

void CreateEntityDialog::renderSpriteSettings() {
    // Выбор текстуры
    if (ImGui::BeginCombo("Texture",
        m_SelectedTextureIndex >= 0 && m_SelectedTextureIndex < m_AvailableTextures.size()
        ? std::filesystem::path(m_AvailableTextures[m_SelectedTextureIndex]).filename().string().c_str()
        : "Select texture...")) {

        if (ImGui::Selectable("None", m_SelectedTextureIndex == -1)) {
            m_SelectedTextureIndex = -1;
            m_SelectedTexturePath.clear();
        }

        for (int i = 0; i < m_AvailableTextures.size(); i++) {
            bool isSelected = (m_SelectedTextureIndex == i);
            std::string filename = std::filesystem::path(m_AvailableTextures[i]).filename().string();

            if (ImGui::Selectable(filename.c_str(), isSelected)) {
                m_SelectedTextureIndex = i;
                m_SelectedTexturePath = m_AvailableTextures[i];
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Цвет
    float col[4] = {
        m_SpriteColor.r / 255.f,
        m_SpriteColor.g / 255.f,
        m_SpriteColor.b / 255.f,
        m_SpriteColor.a / 255.f
    };
    if (ImGui::ColorEdit4("Color", col)) {
        m_SpriteColor = sf::Color(
            static_cast<uint8_t>(col[0] * 255),
            static_cast<uint8_t>(col[1] * 255),
            static_cast<uint8_t>(col[2] * 255),
            static_cast<uint8_t>(col[3] * 255)
        );
    }

    // Слой
    ImGui::DragInt("Layer", &m_SpriteLayer, 1, -100, 100);
}

void CreateEntityDialog::renderVelocitySettings() {
    float vel[2] = { m_InitialVelocity.x, m_InitialVelocity.y };
    if (ImGui::DragFloat2("Initial Velocity", vel, 10.f)) {
        m_InitialVelocity = { vel[0], vel[1] };
    }

    ImGui::DragFloat("Max Speed", &m_MaxSpeed, 10.f, 0.f, 10000.f);
    ImGui::DragFloat("Damping", &m_Damping, 0.01f, 0.f, 1.f);

    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
        "Damping reduces velocity over time (0 = none)");
}

void CreateEntityDialog::renderColliderSettings() {
    const char* shapes[] = { "Box", "Circle" };
    int currentShape = static_cast<int>(m_ColliderShape);

    if (ImGui::Combo("Shape", &currentShape, shapes, 2)) {
        m_ColliderShape = static_cast<ColliderComponent::Shape>(currentShape);
    }

    if (m_ColliderShape == ColliderComponent::Shape::Box) {
        float size[2] = { m_BoxSize.x, m_BoxSize.y };
        if (ImGui::DragFloat2("Size", size, 1.f, 1.f, 1000.f)) {
            m_BoxSize = { size[0], size[1] };
        }
    }
    else {
        ImGui::DragFloat("Radius", &m_CircleRadius, 1.f, 1.f, 1000.f);
    }

    float offset[2] = { m_ColliderOffset.x, m_ColliderOffset.y };
    if (ImGui::DragFloat2("Offset", offset, 1.f)) {
        m_ColliderOffset = { offset[0], offset[1] };
    }

    ImGui::Checkbox("Is Trigger", &m_IsTrigger);
    ImGui::Checkbox("Is Static", &m_IsStatic);
}

void CreateEntityDialog::renderHealthSettings() {
    ImGui::DragInt("Max Health", &m_MaxHealth, 1, 1, 10000);
    ImGui::DragInt("Current Health", &m_CurrentHealth, 1, 0, m_MaxHealth);

    if (m_CurrentHealth > m_MaxHealth) {
        m_CurrentHealth = m_MaxHealth;
    }
}

void CreateEntityDialog::renderTagSettings() {
    ImGui::InputText("Tag", m_TagName, sizeof(m_TagName));

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
        "Common tags: Player, Enemy, Item, Obstacle, UI");

    // Быстрые пресеты
    if (ImGui::Button("Player")) strcpy_s(m_TagName, "Player");
    ImGui::SameLine();
    if (ImGui::Button("Enemy")) strcpy_s(m_TagName, "Enemy");
    ImGui::SameLine();
    if (ImGui::Button("Item")) strcpy_s(m_TagName, "Item");
    ImGui::SameLine();
    if (ImGui::Button("Obstacle")) strcpy_s(m_TagName, "Obstacle");
}

void CreateEntityDialog::createEntity() {
    Entity entity = m_Scene.createEntity();

    // Transform всегда добавляется автоматически в createEntity
    m_Scene.setPosition(entity, m_WorldPosition);
    m_Scene.setRotation(entity, sf::degrees(0.f));
    m_Scene.setScale(entity, { 1.f, 1.f });

    // Sprite
    if (m_AddSprite) {
        if (!m_SelectedTexturePath.empty()) {
            try {
                auto texture = ResourceManager::loadTexture(m_SelectedTexturePath);
                m_Scene.setTexture(entity, texture);

                auto sprite = m_Scene.getSprite(entity);
                if (sprite) {
                    sprite->setColor(m_SpriteColor);
                }

                auto& spriteComp = m_Scene.sprites[entity];
                spriteComp.layer = m_SpriteLayer;
            }
            catch (const std::exception& e) {
                std::cout << "Failed to load texture: " << e.what() << std::endl;
            }
        }
    }

    // Velocity
    if (m_AddVelocity) {
        auto& vel = m_Scene.velocities[entity];
        vel.Velocity = m_InitialVelocity;
        vel.maxSpeed = m_MaxSpeed;
        vel.damping = m_Damping;
    }

    // Collider
    if (m_AddCollider) {
        auto& col = m_Scene.colliders[entity];
        col.shape = m_ColliderShape;
        col.size = m_BoxSize;
        col.radius = m_CircleRadius;
        col.offset = m_ColliderOffset;
        col.isTrigger = m_IsTrigger;
        col.isStatic = m_IsStatic;
    }

    // Health
    if (m_AddHealth) {
        auto& health = m_Scene.healths[entity];
        health.maxHealth = m_MaxHealth;
        health.currentHealth = m_CurrentHealth;
    }

    // Tag
    if (m_AddTag) {
        auto& tag = m_Scene.tags[entity];
        tag.Tag = m_TagName;
    }

    // Вызываем callback если есть
    if (m_OnEntityCreated) {
        m_OnEntityCreated(entity);
    }
}

void CreateEntityDialog::scanTextures() {
    TextureBrowser browser(m_Scene);
    browser.scanAvailableTextures();
    m_AvailableTextures = browser.getAvailableTextures();
}

const char* CreateEntityDialog::getStepName(int step) const {
    switch (step) {
    case 0: return "Basic Info";
    case 1: return "Components";
    case 2: return "Settings";
    default: return "Unknown";
    }
}

bool CreateEntityDialog::canProceed() const {
    if (m_CurrentStep == 0) {
        return strlen(m_EntityName) > 0;
    }
    return true;
}