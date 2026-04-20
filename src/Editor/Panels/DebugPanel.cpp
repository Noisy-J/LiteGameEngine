#include "DebugPanel.hpp"

void DebugPanel::render(Scene& scene, Entity selectedEntity) {
    ImGui::Begin("Debug");

    ImGui::Text("Scene Statistics");
    ImGui::Separator();
    ImGui::Text("Entities: %zu", scene.transforms.size());
    ImGui::Text("Sprites: %zu", scene.sprites.size());
    ImGui::Text("Velocities: %zu", scene.velocities.size());
    ImGui::Text("Colliders: %zu", scene.colliders.size());

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Visualization");

    ImGui::Checkbox("Show Bounds", &m_ShowBounds);
    ImGui::Checkbox("Show Colliders", &m_ShowColliders);
    ImGui::Checkbox("Show Velocity Vectors", &m_ShowVelocity);

    ImGui::Spacing();
    ImGui::Separator();

    if (selectedEntity != INVALID_ENTITY) {
        ImGui::Text("Selected Entity: %d", selectedEntity);

        auto tfIt = scene.transforms.find(selectedEntity);
        if (tfIt != scene.transforms.end()) {
            ImGui::Text("Position: %.2f, %.2f", tfIt->second.Pos.x, tfIt->second.Pos.y);
        }

        auto velIt = scene.velocities.find(selectedEntity);
        if (velIt != scene.velocities.end()) {
            float speed = std::sqrt(velIt->second.Velocity.x * velIt->second.Velocity.x +
                velIt->second.Velocity.y * velIt->second.Velocity.y);
            ImGui::Text("Speed: %.2f", speed);
        }
    }

    ImGui::End();
}