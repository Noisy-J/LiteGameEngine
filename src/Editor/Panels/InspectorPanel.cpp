#include "InspectorPanel.hpp"

InspectorPanel::InspectorPanel(Scene& scene)
    : m_Scene(scene) {
}

void InspectorPanel::render(Entity entity) {
    if (entity == INVALID_ENTITY) {
        ImGui::Text("No entity selected");
        return;
    }

    ImGui::Text("Entity ID: %d", entity);
    ImGui::SameLine();
    ImGui::InputText("##Name", m_EntityNameBuffer, sizeof(m_EntityNameBuffer));

    ImGui::Separator();

    renderTransform(entity);
    renderSprite(entity);
    renderVelocity(entity);

    ImGui::Spacing();
    ImGui::Separator();

    renderAddComponentMenu(entity);

    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::Button("Delete Entity", ImVec2(-1, 0))) {
        m_Scene.destroyEntity(entity);
    }
}

void InspectorPanel::renderTransform(Entity entity) {
    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) return;

    auto tfIt = m_Scene.transforms.find(entity);
    if (tfIt == m_Scene.transforms.end()) return;

    auto& tf = tfIt->second;

    // Position
    float pos[2] = { tf.Pos.x, tf.Pos.y };
    if (ImGui::DragFloat2("Position", pos, 1.f)) {
        tf.Pos = { pos[0], pos[1] };
    }

    // Rotation
    float angle = tf.Rot.asDegrees();
    if (ImGui::DragFloat("Rotation", &angle, 1.f, -360.f, 360.f)) {
        tf.Rot = sf::degrees(angle);
    }

    // Scale
    float scale[2] = { tf.Scale.x, tf.Scale.y };
    if (ImGui::DragFloat2("Scale", scale, 0.1f, 0.01f, 10.f)) {
        tf.Scale = { scale[0], scale[1] };
    }

    if (ImGui::Button("Reset")) {
        tf.Pos = { 0.f, 0.f };
        tf.Rot = sf::degrees(0.f);
        tf.Scale = { 1.f, 1.f };
    }
}

void InspectorPanel::renderSprite(Entity entity) {
    auto spriteIt = m_Scene.sprites.find(entity);

    if (spriteIt != m_Scene.sprites.end()) {
        if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& spriteComp = spriteIt->second;

            // Информация о текстуре
            if (spriteComp.texture) {
                auto texSize = spriteComp.texture->getSize();
                ImGui::Text("Texture: %dx%d", texSize.x, texSize.y);
                ImGui::Text("References: %ld", spriteComp.texture.use_count());
            }

            // Слой
            ImGui::DragInt("Layer", &spriteComp.layer, 1, -100, 100);

            // Видимость
            ImGui::Checkbox("Visible", &spriteComp.visible);

            // Цвет
            if (spriteComp.sprite) {
                sf::Color color = spriteComp.sprite->getColor();
                float col[4] = {
                    color.r / 255.f, color.g / 255.f,
                    color.b / 255.f, color.a / 255.f
                };
                if (ImGui::ColorEdit4("Tint", col)) {
                    spriteComp.sprite->setColor(sf::Color(
                        static_cast<uint8_t>(col[0] * 255),
                        static_cast<uint8_t>(col[1] * 255),
                        static_cast<uint8_t>(col[2] * 255),
                        static_cast<uint8_t>(col[3] * 255)
                    ));
                }
            }

            // Кнопка удаления компонента
            if (ImGui::Button("Remove Sprite Component")) {
                m_Scene.sprites.erase(entity);
            }
        }
    }
    else {
        if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("No sprite component");
            if (ImGui::Button("Add Sprite")) {
                // TODO: Открыть диалог выбора текстуры
            }
        }
    }
}

void InspectorPanel::renderVelocity(Entity entity) {
    auto velIt = m_Scene.velocities.find(entity);

    if (velIt != m_Scene.velocities.end()) {
        if (ImGui::CollapsingHeader("Velocity", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& vel = velIt->second;

            float velocity[2] = { vel.Velocity.x, vel.Velocity.y };
            if (ImGui::DragFloat2("Velocity", velocity, 10.f)) {
                vel.Velocity = { velocity[0], velocity[1] };
            }

            ImGui::DragFloat("Max Speed", &vel.maxSpeed, 10.f, 0.f, 10000.f);
            ImGui::DragFloat("Damping", &vel.damping, 0.01f, 0.f, 1.f);

            if (ImGui::Button("Remove Velocity Component")) {
                m_Scene.velocities.erase(entity);
            }
        }
    }
    else {
        if (ImGui::CollapsingHeader("Velocity", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("No velocity component");
            if (ImGui::Button("Add Velocity")) {
                m_Scene.velocities[entity] = VelocityComponent{};
            }
        }
    }
}

void InspectorPanel::renderAddComponentMenu(Entity entity) {
    ImGui::Text("Add Component");

    if (ImGui::BeginCombo("##AddComponent", "Select component...")) {
        if (!m_Scene.sprites.contains(entity)) {
            if (ImGui::Selectable("Sprite")) {
                // TODO: Открыть диалог
            }
        }

        if (!m_Scene.velocities.contains(entity)) {
            if (ImGui::Selectable("Velocity")) {
                m_Scene.velocities[entity] = VelocityComponent{};
            }
        }

        if (!m_Scene.colliders.contains(entity)) {
            if (ImGui::Selectable("Collider")) {
                m_Scene.colliders[entity] = ColliderComponent{};
            }
        }

        if (!m_Scene.healths.contains(entity)) {
            if (ImGui::Selectable("Health")) {
                m_Scene.healths[entity] = HealthComponent{};
            }
        }

        ImGui::EndCombo();
    }
}