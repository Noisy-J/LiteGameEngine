#pragma once

#include <imgui.h>
#include <vector>
#include <string>
#include <functional>
#include "../../ECS/Scene.hpp"

class HierarchyPanel {
public:
    HierarchyPanel(Scene& scene);
    ~HierarchyPanel() = default;

    void render();

    void setOnEntitySelected(std::function<void(Entity)> callback) { m_OnEntitySelected = callback; }
    void setOnEntityDoubleClicked(std::function<void(Entity)> callback) { m_OnEntityDoubleClicked = callback; }
    void setOnContextMenu(std::function<void(Entity, const sf::Vector2f&)> callback) { m_OnContextMenu = callback; }

    void setSelectedEntity(Entity entity) { m_SelectedEntity = entity; }
    Entity getSelectedEntity() const { return m_SelectedEntity; }

private:
    Scene& m_Scene;
    Entity m_SelectedEntity{ INVALID_ENTITY };

    // Поиск
    char m_SearchBuffer[64] = "";

    // Drag & Drop
    Entity m_DraggedEntity{ INVALID_ENTITY };
    Entity m_DropTargetEntity{ INVALID_ENTITY };
    int m_DropPosition{ -1 }; // -1: на сущность, 0: перед, 1: после

    // Колбэки
    std::function<void(Entity)> m_OnEntitySelected;
    std::function<void(Entity)> m_OnEntityDoubleClicked;
    std::function<void(Entity, const sf::Vector2f&)> m_OnContextMenu;

    // Методы отрисовки
    void renderSceneHeader();
    void renderEntityNode(Entity entity, int depth = 0);
    void renderContextMenu(Entity entity);
    void renderCreateMenu();
    void renderSearchBar();

    bool m_RenamingScene{ false };

    // Вспомогательные методы
    bool matchesSearch(Entity entity) const;
    void handleDragDrop(Entity entity);
    void handleSelection(Entity entity);
    void handleRenaming(Entity entity);

    // Для переименования
    Entity m_RenamingEntity{ INVALID_ENTITY };
    char m_RenameBuffer[128] = "";

    // Группы
    void createGroup(const std::string& name, Entity parent = INVALID_ENTITY);
    void deleteEntity(Entity entity);
    void duplicateEntity(Entity entity);

    std::string getEntityDisplayName(Entity entity) const;
};