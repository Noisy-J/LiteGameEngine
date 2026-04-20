#include "EditorUI.hpp"
#include "../Resources/ResourceManager.hpp"
#include "../Utils/CoordinateConverter.hpp"

EditorUI::EditorUI(sf::RenderWindow& window, Scene& scene, Viewport& viewport)
    : m_Window(window)
    , m_Scene(scene)
    , m_Viewport(viewport) {

    m_InspectorPanel = std::make_unique<InspectorPanel>(scene);
    m_DebugPanel = std::make_unique<DebugPanel>();
    m_ViewportPanel = std::make_unique<ViewportPanel>(viewport);
    m_ContentBrowserPanel = std::make_unique<ContentBrowserPanel>();
    m_TextureDialog = std::make_unique<TextureSelectorDialog>(scene);
    m_CreateEntityDialog = std::make_unique<CreateEntityDialog>(scene);

    // Устанавливаем callback для создания сущности
    m_CreateEntityDialog->setOnEntityCreated([this](Entity entity) {
        m_SelectedEntity = entity;
        });
}

void EditorUI::update(Entity selectedEntity, float deltaTime) {
    m_SelectedEntity = selectedEntity;
}

void EditorUI::render() {
    renderMainMenu();

    // Inspector
    ImGui::Begin("Inspector");
    m_InspectorPanel->render(m_SelectedEntity);
    ImGui::End();

    // Content Browser
    m_ContentBrowserPanel->render();

    // Debug Panel
    m_DebugPanel->render(m_Scene, m_SelectedEntity);

    // Viewport
    m_ViewportPanel->render();

    // Контекстное меню
    if (m_ShowContextMenu) {
        renderContextMenu();
    }

    // Диалоги
    m_TextureDialog->render();
    m_CreateEntityDialog->render();  // Добавлено
}

void EditorUI::renderMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene")) {}
            if (ImGui::MenuItem("Save Scene")) {}
            if (ImGui::MenuItem("Load Scene")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
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

        if (ImGui::BeginMenu("GameObject")) {  // Добавлено меню
            if (ImGui::MenuItem("Create Empty")) {
                // Создать в центре viewport
                sf::Vector2f center = m_Viewport.getView().getCenter();
                showCreateEntityDialog(center);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Inspector", nullptr, true);
            ImGui::MenuItem("Content Browser", nullptr, true);
            ImGui::MenuItem("Debug Panel", nullptr, true);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {}
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}
void EditorUI::renderContextMenu() {
    ImGui::OpenPopup("ViewportContextMenu");
    m_ShowContextMenu = false;
    
    if (ImGui::BeginPopup("ViewportContextMenu")) {
        sf::Vector2f worldPos = CoordinateConverter::screenToWorld(
            sf::Vector2i(static_cast<int>(m_ContextMenuPos.x), 
                        static_cast<int>(m_ContextMenuPos.y)),
            m_Window, m_Viewport);
        
        // Заголовок
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Create");
        ImGui::Separator();
        
        if (ImGui::MenuItem("Create Empty Entity")) {
            Entity newEntity = m_Scene.createEntity();
            m_Scene.setPosition(newEntity, worldPos);
            m_SelectedEntity = newEntity;
        }
        
        if (ImGui::MenuItem("Create with Wizard...")) {
            showCreateEntityDialog(worldPos);
        }
        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "2D Object");
        ImGui::Separator();
        
        if (ImGui::MenuItem("Sprite")) {
            Entity entity = m_Scene.createEntity();
            m_Scene.setPosition(entity, worldPos);
            m_SelectedEntity = entity;
            showTextureBrowser(entity);
        }
        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Physics");
        ImGui::Separator();
        
        if (ImGui::BeginMenu("Collider")) {
            if (ImGui::MenuItem("Box Collider")) {
                Entity entity = m_Scene.createEntity();
                m_Scene.setPosition(entity, worldPos);
                auto& col = m_Scene.colliders[entity];
                col.shape = ColliderComponent::Shape::Box;
                col.size = {64.f, 64.f};
                m_SelectedEntity = entity;
            }
            
            if (ImGui::MenuItem("Circle Collider")) {
                Entity entity = m_Scene.createEntity();
                m_Scene.setPosition(entity, worldPos);
                auto& col = m_Scene.colliders[entity];
                col.shape = ColliderComponent::Shape::Circle;
                col.radius = 32.f;
                m_SelectedEntity = entity;
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::MenuItem("Rigidbody (Physics)")) {
            Entity entity = m_Scene.createEntity();
            m_Scene.setPosition(entity, worldPos);
            m_Scene.velocities[entity] = VelocityComponent{};
            m_SelectedEntity = entity;
        }
        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Prefabs");
        ImGui::Separator();
        
        if (ImGui::MenuItem("Player")) {
            Entity entity = m_Scene.createEntity();
            m_Scene.setPosition(entity, worldPos);
            
            // Добавляем спрайт если есть текстура по умолчанию
            try {
                auto texture = ResourceManager::loadTexture("./assets/hero.png");
                m_Scene.setTexture(entity, texture);
            } catch (...) {}
            
            m_Scene.setVelocity(entity, {300.f, 0.f});
            m_Scene.tags[entity].Tag = "Player";
            m_SelectedEntity = entity;
        }
        
        if (ImGui::MenuItem("Enemy")) {
            Entity entity = m_Scene.createEntity();
            m_Scene.setPosition(entity, worldPos);
            m_Scene.setVelocity(entity, {150.f, 0.f});
            
            auto& health = m_Scene.healths[entity];
            health.maxHealth = 50;
            health.currentHealth = 50;
            
            m_Scene.tags[entity].Tag = "Enemy";
            m_SelectedEntity = entity;
        }
        
        if (ImGui::MenuItem("Item")) {
            Entity entity = m_Scene.createEntity();
            m_Scene.setPosition(entity, worldPos);
            m_Scene.tags[entity].Tag = "Item";
            
            auto& col = m_Scene.colliders[entity];
            col.isTrigger = true;
            col.size = {32.f, 32.f};
            
            m_SelectedEntity = entity;
        }
        
        if (ImGui::MenuItem("Camera Target")) {
            Entity entity = m_Scene.createEntity();
            m_Scene.setPosition(entity, worldPos);
            m_Scene.tags[entity].Tag = "CameraTarget";
            m_SelectedEntity = entity;
        }
        
        ImGui::Separator();
        
        if (ImGui::MenuItem("Paste", "Ctrl+V", false, false)) {
            // TODO: Реализовать вставку
        }
        
        ImGui::EndPopup();
    }
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
