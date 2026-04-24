#include "EditorUI.hpp"
#include "../Resources/ResourceManager.hpp"
#include "../Utils/CoordinateConverter.hpp"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

EditorUI::EditorUI(sf::RenderWindow& window, Scene& scene, Viewport& viewport)
    : m_Window(window)
    , m_Scene(scene)
    , m_Viewport(viewport) {

    m_InspectorPanel = std::make_unique<InspectorPanel>(scene);
    m_DebugPanel = std::make_unique<DebugPanel>();
    m_ViewportPanel = std::make_unique<ViewportPanel>(viewport);
    m_ContentBrowserPanel = std::make_unique<ContentBrowserPanel>();
    m_HierarchyPanel = std::make_unique<HierarchyPanel>(scene);
    m_ScriptEditorPanel = std::make_unique<ScriptEditorPanel>(scene);
    m_SceneSerializer = std::make_unique<SceneSerializer>(scene);
    m_TextureDialog = std::make_unique<TextureSelectorDialog>(scene);
    m_CreateEntityDialog = std::make_unique<CreateEntityDialog>(scene);
    m_FileDialog = std::make_unique<FileDialog>();

    // Колбэк для FileDialog
    m_FileDialog->setOnFileSelected([this](const std::string& path) {
        if (m_PendingSave) {
            fs::path filePath(path);
            m_Scene.sceneName = filePath.stem().string();
            m_SceneSerializer->save(path);
            m_PendingSave = false;
        }
        else {
            m_SceneSerializer->load(path);
            m_SelectedEntity = INVALID_ENTITY;
        }
        });

    // Колбэк для CreateEntityDialog
    m_CreateEntityDialog->setOnEntityCreated([this](Entity entity) {
        m_SelectedEntity = entity;
        });

    // Колбэк для Inspector - добавление спрайта
    m_InspectorPanel->setOnAddSprite([this](Entity entity) {
        if (!m_Scene.sprites.contains(entity)) {
            m_Scene.sprites[entity] = SpriteComponent{};
        }
        m_TextureDialog->open(entity);
        });

    // Колбэк для Inspector - открытие редактора скриптов
    m_InspectorPanel->setOnOpenScriptEditor([this](Entity entity) {
        openScriptEditor(entity);
        });

    // Колбэк для Hierarchy
    m_HierarchyPanel->setOnEntitySelected([this](Entity entity) {
        m_SelectedEntity = entity;
        });

    m_HierarchyPanel->setOnEntityDoubleClicked([this](Entity entity) {
        // Фокус на сущности во вьюпорте
        m_SelectedEntity = entity;
        });

    // Колбэк для Content Browser - двойной клик по текстуре
    m_ContentBrowserPanel->setOnAssetDoubleClicked([this](const std::string& path) {
        Entity entity = m_Scene.createEntity();
        sf::Vector2f center = m_Viewport.getView().getCenter();
        m_Scene.setPosition(entity, center);
        m_Scene.setEntityName(entity, fs::path(path).stem().string());
        try {
            auto texture = ResourceManager::loadTexture(path);
            m_Scene.setTexture(entity, texture, path);
        }
        catch (...) {}
        m_SelectedEntity = entity;
        });
}

void EditorUI::openScriptEditor(Entity entity) {
    m_ShowScriptEditor = true;
    m_ScriptEditorPanel->openForEntity(entity);
}

void EditorUI::update(Entity selectedEntity, float deltaTime) {
    m_SelectedEntity = selectedEntity;
    m_HierarchyPanel->setSelectedEntity(selectedEntity);
}

void EditorUI::render() {
    renderMainMenu();

    // Иерархия
    if (m_ShowHierarchy) {
        m_HierarchyPanel->render();
    }

    // Инспектор
    if (m_ShowInspector) {
        ImGui::Begin("Inspector");
        m_InspectorPanel->render(m_SelectedEntity);
        ImGui::End();
    }

    // Content Browser
    if (m_ShowContentBrowser) {
        m_ContentBrowserPanel->render();
    }

    // Debug Panel
    if (m_ShowDebug) {
        m_DebugPanel->render(m_Scene, m_SelectedEntity);
    }

    // Viewport
    m_ViewportPanel->render();

    // Script Editor
    if (m_ShowScriptEditor) {
        m_ScriptEditorPanel->render();
    }

    // Диалоги
    m_TextureDialog->render();
    m_CreateEntityDialog->render();
    m_FileDialog->render();

    // Контекстное меню
    if (m_ShowContextMenu) {
        renderContextMenu();
    }
}

void EditorUI::renderMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                m_SceneSerializer->newScene();
                m_Scene.sceneName = "New Scene";
                m_SelectedEntity = INVALID_ENTITY;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) {
                openSceneDialog();
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                saveSceneDialog();
            }
            if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) {
                saveSceneDialog();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                m_Window.close();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Delete", "Del")) {
                if (m_SelectedEntity != INVALID_ENTITY) {
                    m_Scene.destroyEntity(m_SelectedEntity);
                    m_SelectedEntity = INVALID_ENTITY;
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("GameObject")) {
            if (ImGui::MenuItem("Create Empty")) {
                Entity entity = m_Scene.createEntity();
                m_Scene.setEntityName(entity, "New Entity");
                m_SelectedEntity = entity;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Hierarchy", nullptr, &m_ShowHierarchy);
            ImGui::MenuItem("Inspector", nullptr, &m_ShowInspector);
            ImGui::MenuItem("Content Browser", nullptr, &m_ShowContentBrowser);
            ImGui::MenuItem("Debug Panel", nullptr, &m_ShowDebug);
            ImGui::MenuItem("Script Editor", nullptr, &m_ShowScriptEditor);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {}
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void EditorUI::openSceneDialog() {
    m_PendingSave = false;
    m_FileDialog->open(FileDialog::Mode::Open, "Open Scene", "./scenes");
}

void EditorUI::saveSceneDialog() {
    m_PendingSave = true;
    m_FileDialog->open(FileDialog::Mode::Save, "Save Scene", "./scenes");
}

void EditorUI::showTextureBrowser(Entity entity) {
    m_TextureDialog->open(entity);
}

void EditorUI::showContextMenu(const sf::Vector2f& pos) {
    m_ShowContextMenu = true;
    m_ContextMenuPos = pos;
}

void EditorUI::showCreateEntityDialog(const sf::Vector2f& worldPos) {
    m_CreateEntityDialog->open(worldPos);
}

void EditorUI::renderContextMenu() {
    if (!m_ShowContextMenu) return;

    // Устанавливаем позицию попапа в месте клика
    ImGui::SetNextWindowPos(ImVec2(m_ContextMenuPos.x, m_ContextMenuPos.y));

    if (ImGui::Begin("##ViewportContextMenu", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings)) {

        sf::Vector2f worldPos = CoordinateConverter::screenToWorld(
            sf::Vector2i(static_cast<int>(m_ContextMenuPos.x),
                static_cast<int>(m_ContextMenuPos.y)),
            m_Window, m_Viewport);

        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Create");
        ImGui::Separator();

        if (ImGui::MenuItem("Create Empty Entity")) {
            Entity entity = m_Scene.createEntity();
            m_Scene.setEntityName(entity, "Entity");
            m_Scene.setPosition(entity, worldPos);
            m_SelectedEntity = entity;
            m_ShowContextMenu = false;
        }

        if (ImGui::MenuItem("Create Sprite")) {
            Entity entity = m_Scene.createEntity();
            m_Scene.setEntityName(entity, "Sprite");
            m_Scene.setPosition(entity, worldPos);
            showTextureBrowser(entity);
            m_SelectedEntity = entity;
            m_ShowContextMenu = false;
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Scripting");
        ImGui::Separator();

        if (ImGui::MenuItem("Create Script Object")) {
            Entity entity = m_Scene.createEntity();
            m_Scene.setEntityName(entity, "ScriptObject");
            m_Scene.setPosition(entity, worldPos);
            m_Scene.scripts[entity] = ScriptComponent{};
            m_SelectedEntity = entity;
            m_ShowContextMenu = false;
            openScriptEditor(entity);
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Physics");
        ImGui::Separator();

        if (ImGui::MenuItem("Box Collider")) {
            Entity entity = m_Scene.createEntity();
            m_Scene.setEntityName(entity, "Collider");
            m_Scene.setPosition(entity, worldPos);
            auto& col = m_Scene.colliders[entity];
            col.shape = ColliderComponent::Shape::Box;
            col.size = { 64.f, 64.f };
            m_SelectedEntity = entity;
            m_ShowContextMenu = false;
        }

        // Закрываем при клике вне меню
        if (!ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) {
            m_ShowContextMenu = false;
        }

        // Закрываем при нажатии Escape
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            m_ShowContextMenu = false;
        }

        ImGui::End();
    }
    else {
        m_ShowContextMenu = false;
    }
}