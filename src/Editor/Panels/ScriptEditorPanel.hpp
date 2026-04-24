#pragma once

#include <imgui.h>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <algorithm>
#include "../../ECS/Scene.hpp"
#include "../../Scripting/NodeSystem.hpp"

class ScriptEditorPanel {
public:
    ScriptEditorPanel(Scene& scene);
    ~ScriptEditorPanel() = default;

    void render();
    void openForEntity(Entity entity);
    bool isOpen() const { return m_IsOpen; }

private:
    Scene& m_Scene;
    bool m_IsOpen{ false };
    Entity m_CurrentEntity{ INVALID_ENTITY };

    std::vector<Scripting::Node> m_Nodes;
    std::vector<Scripting::Link> m_Links;
    int m_NextNodeId{ 0 };
    int m_NextLinkId{ 0 };

    // Canvas
    ImVec2 m_CanvasOrigin{ 0, 0 };
    ImVec2 m_CanvasOffset{ 0, 0 };
    float m_CanvasScale{ 1.0f };

    // Взаимодействие
    int m_SelectedNodeId{ -1 };
    int m_HoveredNodeId{ -1 };
    int m_HoveredPinId{ -1 };
    int m_HoveredPinNodeId{ -1 };
    bool m_HoveredPinIsInput{ false };

    // Drag&Drop соединений
    bool m_IsDraggingLink{ false };
    int m_DragStartNodeId{ -1 };
    int m_DragStartPinId{ -1 };
    bool m_DragStartIsOutput{ false };
    ImVec2 m_DragEndPos{ 0, 0 };

    enum class NodeAction { None, Select, Move, Connect };
    NodeAction m_CurrentAction{ NodeAction::None };

    void renderToolbar();
    void renderCanvas();
    void renderNode(Scripting::Node& node);
    void renderLinks();
    void renderContextMenu();
    void renderNodeProperties();
    void renderDraggingLink(ImDrawList* drawList);

    void addNode(const std::string& type, ImVec2 canvasPos);
    void deleteNode(int nodeId);
    void createLink(int startNodeId, int startPinId, int endNodeId, int endPinId);
    void removeLink(int linkId);

    void executeGraph();
    void saveGraph();
    void loadGraph();

    // Координаты (используют m_CanvasOrigin)
    ImVec2 screenToCanvas(const ImVec2& screenPos) const;
    ImVec2 canvasToScreen(const ImVec2& canvasPos) const;
    ImVec2 getPinScreenPos(int nodeId, int pinId, bool isInput) const;

    ImU32 getPinColor(Scripting::PinType type) const;
    const char* getPinTypeName(Scripting::PinType type) const;
};