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
#include "Panels/HierarchyPanel.hpp"
#include "Panels/ScriptEditorPanel.hpp"
#include "Dialogs/TextureSelectorDialog.hpp"
#include "Dialogs/CreateEntityDialog.hpp"
#include "Dialogs/FileDialog.hpp"
#include "../Serialization/SceneSerializer.hpp"

class EditorUI {
public:
    EditorUI(sf::RenderWindow& window, Scene& scene, Viewport& viewport);
    ~EditorUI() = default;

    void update(Entity selectedEntity, float deltaTime);
    void render();

    void showTextureBrowser(Entity entity);
    void showContextMenu(const sf::Vector2f& pos);
    void showCreateEntityDialog(const sf::Vector2f& worldPos);
    void openScriptEditor(Entity entity);

private:
    sf::RenderWindow& m_Window;
    Scene& m_Scene;
    Viewport& m_Viewport;

    std::unique_ptr<InspectorPanel> m_InspectorPanel;
    std::unique_ptr<DebugPanel> m_DebugPanel;
    std::unique_ptr<ViewportPanel> m_ViewportPanel;
    std::unique_ptr<ContentBrowserPanel> m_ContentBrowserPanel;
    std::unique_ptr<HierarchyPanel> m_HierarchyPanel;
    std::unique_ptr<ScriptEditorPanel> m_ScriptEditorPanel;
    std::unique_ptr<SceneSerializer> m_SceneSerializer;
    std::unique_ptr<TextureSelectorDialog> m_TextureDialog;
    std::unique_ptr<CreateEntityDialog> m_CreateEntityDialog;
    std::unique_ptr<FileDialog> m_FileDialog;

    Entity m_SelectedEntity{ INVALID_ENTITY };
    bool m_ShowContextMenu{ false };
    sf::Vector2f m_ContextMenuPos{ 0.f, 0.f };
    bool m_PendingSave{ false };

    // Видимость панелей
    bool m_ShowHierarchy{ true };
    bool m_ShowInspector{ true };
    bool m_ShowDebug{ false };
    bool m_ShowContentBrowser{ true };
    bool m_ShowScriptEditor{ false };

    void renderMainMenu();
    void renderContextMenu();
    void openSceneDialog();
    void saveSceneDialog();
};