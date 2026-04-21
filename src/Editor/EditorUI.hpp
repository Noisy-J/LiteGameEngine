#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <imgui.h>
#include "../ECS/Scene.hpp"
#include "../Rendering/Viewport.hpp"
#include "Panels/InspectorPanel.hpp"
#include "Panels/DebugPanel.hpp"
#include "Panels/ViewportPanel.hpp"
#include "Panels/ContentBrowserPanel.hpp"
#include "Dialogs/TextureSelectorDialog.hpp"
#include "Dialogs/CreateEntityDialog.hpp"
#include "Input/InputManager.hpp"

class EditorUI {
public:
    EditorUI(sf::RenderWindow& window, Scene& scene, Viewport& viewport);
    ~EditorUI() = default;

    void update(Entity selectedEntity, float deltaTime);
    void render();

    void showTextureBrowser(Entity entity);
    void showContextMenu(const sf::Vector2f& pos);
    void showCreateEntityDialog(const sf::Vector2f& worldPos);

private:
    sf::RenderWindow& m_Window;
    Scene& m_Scene;
    Viewport& m_Viewport;

    std::unique_ptr<InspectorPanel> m_InspectorPanel;
    std::unique_ptr<DebugPanel> m_DebugPanel;
    std::unique_ptr<ViewportPanel> m_ViewportPanel;
    std::unique_ptr<ContentBrowserPanel> m_ContentBrowserPanel;

    std::unique_ptr<TextureSelectorDialog> m_TextureDialog;
    std::unique_ptr<CreateEntityDialog> m_CreateEntityDialog;

    Entity m_SelectedEntity{ INVALID_ENTITY };
    bool m_ShowContextMenu{ false };
    sf::Vector2f m_ContextMenuPos{ 0.f, 0.f };

    void renderMainMenu();
    void renderContextMenu();
};