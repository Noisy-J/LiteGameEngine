#pragma once
#include "HierarchyPanel.hpp"
#include <iostream>
#include <algorithm>

HierarchyPanel::HierarchyPanel(Scene& scene)
    : m_Scene(scene) {
}

void HierarchyPanel::render() {
    ImGui::Begin("Hierarchy");

    renderSearchBar();
    ImGui::Separator();

    renderSceneHeader();

    ImGui::Separator();

    // Список корневых сущностей
    ImGui::BeginChild("EntityList", ImVec2(0, 0), false);

    auto rootEntities = m_Scene.getRootEntities();
    for (Entity entity : rootEntities) {
        if (matchesSearch(entity)) {
            renderEntityNode(entity);
        }
    }

    // Контекстное меню для пустого места
    if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        renderCreateMenu();
        ImGui::EndPopup();
    }

    // Drag & Drop на пустое место
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY")) {
            Entity draggedEntity = *static_cast<Entity*>(payload->Data);
            m_Scene.removeParent(draggedEntity);
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::EndChild();

    ImGui::End();
}

void HierarchyPanel::renderSceneHeader() {
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.3f, 0.4f, 1.0f));

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;

    // Если переименовываем сцену
    if (m_RenamingEntity == INVALID_ENTITY && m_RenamingScene) {
        ImGui::SetNextItemWidth(-1);
        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##RenameScene", m_RenameBuffer, sizeof(m_RenameBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (strlen(m_RenameBuffer) > 0) {
                m_Scene.sceneName = m_RenameBuffer;
            }
            m_RenamingScene = false;
        }
        if (!ImGui::IsItemActive() && !ImGui::IsItemHovered()) {
            m_RenamingScene = false;
        }
    }
    else {
        bool expanded = ImGui::TreeNodeEx("##Scene", flags, "%s", m_Scene.sceneName.c_str());

        // Контекстное меню сцены
        if (ImGui::BeginPopupContextItem("SceneContext")) {
            if (ImGui::MenuItem("Rename Scene")) {
                strcpy_s(m_RenameBuffer, m_Scene.sceneName.c_str());
                m_RenamingScene = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Create Empty Entity")) {
                Entity newEntity = m_Scene.createEntity();
                m_Scene.setEntityName(newEntity, "New Entity");
                if (m_OnEntitySelected) m_OnEntitySelected(newEntity);
            }
            if (ImGui::MenuItem("Create Sprite")) {
                Entity newEntity = m_Scene.createEntity();
                m_Scene.setEntityName(newEntity, "Sprite");
                if (m_OnEntitySelected) m_OnEntitySelected(newEntity);
            }
            if (ImGui::MenuItem("Create Group")) {
                createGroup("New Group");
            }
            ImGui::EndPopup();
        }

        if (expanded) {
            ImGui::TreePop();
        }
    }

    ImGui::PopStyleColor();
}

void HierarchyPanel::renderEntityNode(Entity entity, int depth) {
    if (!m_Scene.isValid(entity)) return;

    std::string name = getEntityDisplayName(entity);
    bool hasChildren = !m_Scene.getChildren(entity).empty();
    bool isSelected = (m_SelectedEntity == entity);

    float indent = depth * 20.f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
    if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;
    if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

    if (isSelected) {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.7f, 1.0f));
    }

    // Если переименовываем эту сущность - показываем InputText вместо TreeNode
    if (m_RenamingEntity == entity) {
        ImGui::SetNextItemWidth(-1);
        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##Rename", m_RenameBuffer, sizeof(m_RenameBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (strlen(m_RenameBuffer) > 0) {
                m_Scene.setEntityName(entity, m_RenameBuffer);
            }
            m_RenamingEntity = INVALID_ENTITY;
        }
        if (!ImGui::IsItemActive() && !ImGui::IsItemHovered()) {
            m_RenamingEntity = INVALID_ENTITY;
        }
    }
    else {
        bool expanded = ImGui::TreeNodeEx((void*)(uint64_t)entity, flags, "%s", name.c_str());

        if (isSelected) {
            ImGui::PopStyleColor();
        }

        // Drag & Drop
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            ImGui::SetDragDropPayload("HIERARCHY_ENTITY", &entity, sizeof(Entity));
            ImGui::Text("%s", name.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget()) {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY");
            if (payload) {
                Entity draggedEntity = *static_cast<Entity*>(payload->Data);
                if (draggedEntity != entity) {
                    m_Scene.setParent(draggedEntity, entity);
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsItemClicked()) {
            handleSelection(entity);
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            // Двойной клик - переименование
            std::string currentName = getEntityDisplayName(entity);
            strcpy_s(m_RenameBuffer, currentName.c_str());
            m_RenamingEntity = entity;
        }

        renderContextMenu(entity);

        if (expanded) {
            auto children = m_Scene.getChildren(entity);
            for (Entity child : children) {
                if (m_Scene.isValid(child) && matchesSearch(child)) {
                    renderEntityNode(child, depth + 1);
                }
            }
            ImGui::TreePop();
        }
    }
}

void HierarchyPanel::renderContextMenu(Entity entity) {
    std::string popupName = "EntityContext_" + std::to_string(entity);

    if (ImGui::BeginPopupContextItem(popupName.c_str())) {
        if (ImGui::MenuItem("Rename (F2)")) {
            std::string currentName = getEntityDisplayName(entity);
            strcpy_s(m_RenameBuffer, currentName.c_str());
            m_RenamingEntity = entity;
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {
            duplicateEntity(entity);
        }

        if (ImGui::MenuItem("Delete", "Del")) {
            deleteEntity(entity);
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Empty Child")) {
                Entity child = m_Scene.createEntity();
                m_Scene.setEntityName(child, "New Entity");
                m_Scene.setParent(child, entity);
                if (m_OnEntitySelected) m_OnEntitySelected(child);
            }
            if (ImGui::MenuItem("Group")) {
                createGroup("New Group", entity);
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Unparent")) {
            m_Scene.removeParent(entity);
        }

        ImGui::EndPopup();
    }
}

void HierarchyPanel::renderCreateMenu() {
    if (ImGui::MenuItem("Create Empty Entity")) {
        Entity newEntity = m_Scene.createEntity();
        m_Scene.setEntityName(newEntity, "New Entity");
        if (m_OnEntitySelected) m_OnEntitySelected(newEntity);
        // Начинаем переименование
        strcpy_s(m_RenameBuffer, "New Entity");
        m_RenamingEntity = newEntity;
    }

    if (ImGui::MenuItem("Create Sprite")) {
        Entity newEntity = m_Scene.createEntity();
        m_Scene.setEntityName(newEntity, "Sprite");
        if (m_OnEntitySelected) m_OnEntitySelected(newEntity);
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Create Group")) {
        createGroup("New Group");
    }
}

void HierarchyPanel::renderSearchBar() {
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputTextWithHint("##Search", "Search...", m_SearchBuffer, sizeof(m_SearchBuffer))) {
        // Фильтрация происходит автоматически
    }
}

bool HierarchyPanel::matchesSearch(Entity entity) const {
    if (strlen(m_SearchBuffer) == 0) return true;

    std::string name = m_Scene.getEntityName(entity);
    std::string search = m_SearchBuffer;

    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);

    return name.find(search) != std::string::npos;
}

void HierarchyPanel::handleSelection(Entity entity) {
    m_SelectedEntity = entity;
    if (m_OnEntitySelected) {
        m_OnEntitySelected(entity);
    }
}

void HierarchyPanel::createGroup(const std::string& name, Entity parent) {
    Entity group = m_Scene.createEntity();
    m_Scene.setEntityName(group, name);

    if (parent != INVALID_ENTITY) {
        m_Scene.setParent(group, parent);
    }

    if (m_OnEntitySelected) m_OnEntitySelected(group);
}

void HierarchyPanel::deleteEntity(Entity entity) {
    // Удаляем всех потомков
    std::vector<Entity> toDelete = { entity };
    auto children = m_Scene.getChildren(entity);
    for (Entity child : children) {
        toDelete.push_back(child);
    }

    for (Entity e : toDelete) {
        m_Scene.destroyEntity(e);
    }

    if (m_SelectedEntity == entity) {
        m_SelectedEntity = INVALID_ENTITY;
    }
}

void HierarchyPanel::duplicateEntity(Entity entity) {
    Entity copy = m_Scene.createEntity();

    // Копируем трансформ
    m_Scene.setPosition(copy, m_Scene.getPosition(entity));
    m_Scene.setRotation(copy, m_Scene.getRotation(entity));
    m_Scene.setScale(copy, m_Scene.getScale(entity));

    // Копируем имя
    m_Scene.setEntityName(copy, m_Scene.getEntityName(entity) + " (Copy)");

    if (m_OnEntitySelected) m_OnEntitySelected(copy);
}

std::string HierarchyPanel::getEntityDisplayName(Entity entity) const {
    return m_Scene.getEntityName(entity);
}