#include "ScriptEditorPanel.hpp"
#include <iostream>
#include <sstream>
#include <cmath>

ScriptEditorPanel::ScriptEditorPanel(Scene& scene) : m_Scene(scene) {}

void ScriptEditorPanel::openForEntity(Entity entity) {
    m_IsOpen = true;
    m_CurrentEntity = entity;

    if (m_Scene.scripts.contains(entity) && !m_Scene.scripts[entity].scriptPath.empty()) {
        loadGraph();
    }
    else {
        m_Nodes.clear();
        m_Links.clear();
        m_NextNodeId = 0;
        m_NextLinkId = 0;
        addNode("EventBeginPlay", ImVec2(100, 100));
    }
}

void ScriptEditorPanel::render() {
    if (!m_IsOpen) return;

    ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);

    std::string title = "Script Editor - " + m_Scene.getEntityName(m_CurrentEntity);
    if (ImGui::Begin(title.c_str(), &m_IsOpen)) {
        renderToolbar();
        ImGui::Separator();

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() - 250);
        renderCanvas();
        ImGui::NextColumn();
        renderNodeProperties();
        ImGui::Columns(1);
    }
    ImGui::End();
}

void ScriptEditorPanel::renderToolbar() {
    if (ImGui::Button("Save")) saveGraph();
    ImGui::SameLine();
    if (ImGui::Button("Execute")) executeGraph();
    ImGui::SameLine();
    ImGui::Text("Nodes: %zu | Links: %zu", m_Nodes.size(), m_Links.size());
}

void ScriptEditorPanel::renderCanvas() {
    ImGui::BeginChild("Canvas", ImVec2(0, 0), true,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // Сохраняем позицию канваса
    m_CanvasOrigin = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    ImVec2 mouseScreen = ImGui::GetMousePos();
    ImVec2 mouseCanvas = screenToCanvas(mouseScreen);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Сетка
    float gridSize = 20.0f * m_CanvasScale;
    for (float x = fmodf(m_CanvasOffset.x, gridSize); x < canvasSize.x; x += gridSize)
        drawList->AddLine(ImVec2(m_CanvasOrigin.x + x, m_CanvasOrigin.y),
            ImVec2(m_CanvasOrigin.x + x, m_CanvasOrigin.y + canvasSize.y), IM_COL32(60, 60, 60, 40));
    for (float y = fmodf(m_CanvasOffset.y, gridSize); y < canvasSize.y; y += gridSize)
        drawList->AddLine(ImVec2(m_CanvasOrigin.x, m_CanvasOrigin.y + y),
            ImVec2(m_CanvasOrigin.x + canvasSize.x, m_CanvasOrigin.y + y), IM_COL32(60, 60, 60, 40));

    // Позиция мыши в canvas
    char mouseText[64];
    snprintf(mouseText, sizeof(mouseText), "Mouse: %.0f, %.0f", mouseCanvas.x, mouseCanvas.y);
    drawList->AddText(ImVec2(m_CanvasOrigin.x + 5, m_CanvasOrigin.y + canvasSize.y - 20), IM_COL32(150, 150, 150, 255), mouseText);

    // Связи
    renderLinks();

    // Перетаскиваемая связь
    if (m_IsDraggingLink) {
        renderDraggingLink(drawList);
    }

    // Ноды
    m_HoveredPinId = -1;
    m_HoveredPinNodeId = -1;
    m_HoveredNodeId = -1;

    for (auto& node : m_Nodes) {
        renderNode(node);
    }

    // Панорамирование (ПКМ или СКМ)
    if ((ImGui::IsMouseDragging(1) || ImGui::IsMouseDragging(2)) && ImGui::IsWindowHovered()) {
        m_CanvasOffset.x += ImGui::GetIO().MouseDelta.x;
        m_CanvasOffset.y += ImGui::GetIO().MouseDelta.y;
    }

    // Зум к позиции мыши
    if (ImGui::GetIO().MouseWheel != 0 && ImGui::IsWindowHovered()) {
        float oldScale = m_CanvasScale;
        m_CanvasScale += ImGui::GetIO().MouseWheel * 0.1f;
        m_CanvasScale = std::max(0.2f, std::min(m_CanvasScale, 4.0f));

        // Корректируем offset чтобы сохранить позицию под мышью
        m_CanvasOffset.x = mouseScreen.x - m_CanvasOrigin.x - mouseCanvas.x * m_CanvasScale;
        m_CanvasOffset.y = mouseScreen.y - m_CanvasOrigin.y - mouseCanvas.y * m_CanvasScale;
    }

    // Контекстное меню
    renderContextMenu();

    ImGui::EndChild();
}

void ScriptEditorPanel::renderNode(Scripting::Node& node) {
    ImVec2 nodeScreenPos = canvasToScreen(ImVec2(node.PosX, node.PosY));

    float nodeWidth = 200;
    float headerHeight = 28;
    float pinHeight = 24;
    float maxPins = static_cast<float>(std::max(node.InputPins.size(), node.OutputPins.size()));
    float nodeHeight = headerHeight + maxPins * pinHeight + 8;

    ImVec2 nodeEnd(nodeScreenPos.x + nodeWidth, nodeScreenPos.y + nodeHeight);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    bool isSelected = (m_SelectedNodeId == node.Id);

    // Тень
    drawList->AddRectFilled(ImVec2(nodeScreenPos.x + 3, nodeScreenPos.y + 3),
        ImVec2(nodeEnd.x + 3, nodeEnd.y + 3), IM_COL32(0, 0, 0, 100));

    // Фон
    ImU32 bgColor = isSelected ? IM_COL32(70, 70, 95, 240) : IM_COL32(55, 55, 65, 240);
    drawList->AddRectFilled(nodeScreenPos, nodeEnd, bgColor);
    drawList->AddRect(nodeScreenPos, nodeEnd, IM_COL32(100, 100, 130, 255));

    // Заголовок
    drawList->AddRectFilled(nodeScreenPos, ImVec2(nodeEnd.x, nodeScreenPos.y + headerHeight),
        IM_COL32(80, 40, 100, 255));
    drawList->AddText(ImVec2(nodeScreenPos.x + 10, nodeScreenPos.y + 5),
        IM_COL32(255, 255, 255, 255), node.Name.c_str());

    // Входные пины
    for (size_t i = 0; i < node.InputPins.size(); i++) {
        auto& pin = node.InputPins[i];
        float y = nodeScreenPos.y + headerHeight + 4 + i * pinHeight;

        ImVec2 pinCenter(nodeScreenPos.x + 8, y + pinHeight / 2);
        drawList->AddCircleFilled(pinCenter, 7, getPinColor(pin.Type));
        drawList->AddCircle(pinCenter, 7, IM_COL32(0, 0, 0, 200));
        drawList->AddCircle(pinCenter, 5, getPinColor(pin.Type));

        drawList->AddText(ImVec2(nodeScreenPos.x + 20, y + 3),
            IM_COL32(220, 220, 220, 255), pin.Name.c_str());

        // Зона клика
        ImGui::SetCursorScreenPos(ImVec2(nodeScreenPos.x - 4, y - 2));
        ImGui::InvisibleButton(("##in_" + std::to_string(node.Id) + "_" + std::to_string(pin.Id)).c_str(), ImVec2(28, pinHeight));

        if (ImGui::IsItemHovered()) {
            m_HoveredPinId = static_cast<int>(i);
            m_HoveredPinNodeId = node.Id;
            m_HoveredPinIsInput = true;
        }

        // Принимаем связь
        if (ImGui::BeginDragDropTarget()) {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCRIPT_PIN");
            if (payload) {
                int* data = static_cast<int*>(payload->Data);
                int startNodeId = data[0];
                int startPinId = data[1];
                createLink(startNodeId, startPinId, node.Id, pin.Id);
            }
            ImGui::EndDragDropTarget();
        }

        // Клик для начала связи
        if (ImGui::IsItemClicked() && !m_IsDraggingLink) {
            m_IsDraggingLink = true;
            m_DragStartNodeId = node.Id;
            m_DragStartPinId = pin.Id;
            m_DragStartIsOutput = false;
            m_DragEndPos = ImGui::GetMousePos();
        }
    }

    // Выходные пины
    for (size_t i = 0; i < node.OutputPins.size(); i++) {
        auto& pin = node.OutputPins[i];
        float y = nodeScreenPos.y + headerHeight + 4 + i * pinHeight;

        float textWidth = ImGui::CalcTextSize(pin.Name.c_str()).x;
        drawList->AddText(ImVec2(nodeEnd.x - textWidth - 28, y + 3),
            IM_COL32(220, 220, 220, 255), pin.Name.c_str());

        ImVec2 pinCenter(nodeEnd.x - 8, y + pinHeight / 2);
        drawList->AddCircleFilled(pinCenter, 7, getPinColor(pin.Type));
        drawList->AddCircle(pinCenter, 7, IM_COL32(0, 0, 0, 200));
        drawList->AddCircleFilled(pinCenter, 5, getPinColor(pin.Type));

        // Зона клика
        ImGui::SetCursorScreenPos(ImVec2(nodeEnd.x - 24, y - 2));
        ImGui::InvisibleButton(("##out_" + std::to_string(node.Id) + "_" + std::to_string(pin.Id)).c_str(), ImVec2(28, pinHeight));

        if (ImGui::IsItemHovered()) {
            m_HoveredPinId = static_cast<int>(i);
            m_HoveredPinNodeId = node.Id;
            m_HoveredPinIsInput = false;
        }

        // Начинаем связь
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            int payload[2] = { node.Id, pin.Id };
            ImGui::SetDragDropPayload("SCRIPT_PIN", payload, sizeof(payload));
            ImGui::Text("%s", pin.Name.c_str());
            ImGui::EndDragDropSource();
        }

        // Клик для начала связи
        if (ImGui::IsItemClicked() && !m_IsDraggingLink) {
            m_IsDraggingLink = true;
            m_DragStartNodeId = node.Id;
            m_DragStartPinId = pin.Id;
            m_DragStartIsOutput = true;
            m_DragEndPos = ImGui::GetMousePos();
        }
    }

    // Перетаскивание ноды
    ImGui::SetCursorScreenPos(nodeScreenPos);
    ImGui::InvisibleButton(("##node_" + std::to_string(node.Id)).c_str(), ImVec2(nodeWidth, nodeHeight));

    if (ImGui::IsItemHovered()) {
        m_HoveredNodeId = node.Id;

        // Создание связи кликом по выходному пину
        if (m_IsDraggingLink && ImGui::IsMouseReleased(0) && m_DragStartIsOutput) {
            // Ищем входной пин под мышью
            if (m_HoveredPinId >= 0 && m_HoveredPinNodeId != m_DragStartNodeId && m_HoveredPinIsInput) {
                createLink(m_DragStartNodeId, m_DragStartPinId, m_HoveredPinNodeId,
                    m_HoveredPinNodeId >= 0 ? m_HoveredPinNodeId : 0);
            }
            m_IsDraggingLink = false;
        }
    }

    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
        node.PosX += ImGui::GetIO().MouseDelta.x / m_CanvasScale;
        node.PosY += ImGui::GetIO().MouseDelta.y / m_CanvasScale;
    }

    if (ImGui::IsItemClicked()) {
        m_SelectedNodeId = node.Id;
    }
}

void ScriptEditorPanel::renderLinks() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    for (size_t i = 0; i < m_Links.size(); i++) {
        const auto& link = m_Links[i];
        ImVec2 startPos = getPinScreenPos(link.StartNodeId, link.StartPinId, false);
        ImVec2 endPos = getPinScreenPos(link.EndNodeId, link.EndPinId, true);

        if (startPos.x > 0 && endPos.x > 0) {
            // Точка для удаления
            ImVec2 midPos((startPos.x + endPos.x) * 0.5f, (startPos.y + endPos.y) * 0.5f);
            ImGui::SetCursorScreenPos(ImVec2(midPos.x - 5, midPos.y - 5));
            ImGui::InvisibleButton(("##link_" + std::to_string(link.Id)).c_str(), ImVec2(10, 10));
            if (ImGui::IsItemClicked()) {
                removeLink(link.Id);
                return;
            }

            // Кривая
            float dist = fabsf(endPos.x - startPos.x) * 0.5f;
            ImVec2 cp1(startPos.x + dist, startPos.y);
            ImVec2 cp2(endPos.x - dist, endPos.y);
            drawList->AddBezierCubic(startPos, cp1, cp2, endPos, IM_COL32(200, 200, 200, 255), 2.5f);
        }
    }
}

void ScriptEditorPanel::renderDraggingLink(ImDrawList* drawList) {
    if (!m_IsDraggingLink) return;

    ImVec2 startPos = getPinScreenPos(m_DragStartNodeId, m_DragStartPinId, !m_DragStartIsOutput);
    ImVec2 endPos = ImGui::GetMousePos();

    if (startPos.x > 0) {
        float dist = fabsf(endPos.x - startPos.x) * 0.5f;
        ImVec2 cp1(startPos.x + dist, startPos.y);
        ImVec2 cp2(endPos.x - dist, endPos.y);
        drawList->AddBezierCubic(startPos, cp1, cp2, endPos, IM_COL32(255, 200, 100, 200), 2.5f);
    }

    // Отпускаем — проверяем попали ли на входной пин
    if (ImGui::IsMouseReleased(0)) {
        if (m_DragStartIsOutput && m_HoveredPinId >= 0 && m_HoveredPinNodeId >= 0 && m_HoveredPinIsInput) {
            if (m_HoveredPinNodeId != m_DragStartNodeId) {
                // Находим ID пина
                for (auto& node : m_Nodes) {
                    if (node.Id == m_HoveredPinNodeId && m_HoveredPinId < static_cast<int>(node.InputPins.size())) {
                        createLink(m_DragStartNodeId, m_DragStartPinId, m_HoveredPinNodeId, node.InputPins[m_HoveredPinId].Id);
                        break;
                    }
                }
            }
        }
        m_IsDraggingLink = false;
    }
}

ImVec2 ScriptEditorPanel::getPinScreenPos(int nodeId, int pinId, bool isInput) const {
    for (const auto& node : m_Nodes) {
        if (node.Id != nodeId) continue;

        ImVec2 nodeScreen = canvasToScreen(ImVec2(node.PosX, node.PosY));
        float headerHeight = 28;
        float pinHeight = 24;
        float nodeWidth = 200;

        const auto& pins = isInput ? node.InputPins : node.OutputPins;
        for (size_t i = 0; i < pins.size(); i++) {
            if (pins[i].Id == pinId) {
                float y = nodeScreen.y + headerHeight + 4 + i * pinHeight + pinHeight / 2;
                float x = isInput ? nodeScreen.x + 8 : nodeScreen.x + nodeWidth - 8;
                return ImVec2(x, y);
            }
        }
    }
    return ImVec2(-1, -1);
}

void ScriptEditorPanel::renderContextMenu() {
    if (ImGui::BeginPopupContextWindow("CanvasContext")) {
        auto nodes = Scripting::NodeRegistry::Instance().GetAvailableNodes();
        std::string lastCategory;

        for (const auto& [type, path] : nodes) {
            size_t slash = path.find('/');
            std::string cat = path.substr(0, slash);
            std::string name = path.substr(slash + 1);

            if (cat != lastCategory) {
                if (!lastCategory.empty()) ImGui::Separator();
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", cat.c_str());
                lastCategory = cat;
            }

            if (ImGui::MenuItem(name.c_str())) {
                ImVec2 mouseCanvas = screenToCanvas(ImGui::GetMousePos());
                addNode(type, mouseCanvas);
            }
        }

        ImGui::EndPopup();
    }
}
void ScriptEditorPanel::renderNodeProperties() {
    ImGui::BeginChild("Properties", ImVec2(0, 0), true);
    ImGui::Text("Node Properties");
    ImGui::Separator();

    if (m_SelectedNodeId >= 0) {
        for (auto& node : m_Nodes) {
            if (node.Id != m_SelectedNodeId) continue;

            ImGui::Text("Name: %s", node.Name.c_str());
            ImGui::Text("Type: %s", node.Type.c_str());
            ImGui::Text("ID: %d", node.Id);
            ImGui::Text("Position: %.0f, %.0f", node.PosX, node.PosY);

            ImGui::Separator();
            if (ImGui::Button("Delete Node")) deleteNode(node.Id);
            break;
        }
    }
    else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Select a node");
    }
    ImGui::EndChild();
}

void ScriptEditorPanel::addNode(const std::string& type, ImVec2 canvasPos) {
    Scripting::Node node = Scripting::NodeRegistry::Instance().CreateNode(type);
    node.Id = m_NextNodeId++;
    node.PosX = canvasPos.x;
    node.PosY = canvasPos.y;
    m_Nodes.push_back(node);
    m_SelectedNodeId = node.Id;
}

void ScriptEditorPanel::deleteNode(int nodeId) {
    m_Nodes.erase(std::remove_if(m_Nodes.begin(), m_Nodes.end(),
        [nodeId](const Scripting::Node& n) { return n.Id == nodeId; }), m_Nodes.end());
    m_Links.erase(std::remove_if(m_Links.begin(), m_Links.end(),
        [nodeId](const Scripting::Link& l) { return l.StartNodeId == nodeId || l.EndNodeId == nodeId; }), m_Links.end());
    if (m_SelectedNodeId == nodeId) m_SelectedNodeId = -1;
}

void ScriptEditorPanel::createLink(int startNodeId, int startPinId, int endNodeId, int endPinId) {
    // Проверяем что связь не дублируется
    for (const auto& link : m_Links) {
        if (link.StartNodeId == startNodeId && link.StartPinId == startPinId &&
            link.EndNodeId == endNodeId && link.EndPinId == endPinId) return;
    }
    m_Links.push_back({ m_NextLinkId++, startNodeId, startPinId, endNodeId, endPinId });
}

void ScriptEditorPanel::removeLink(int linkId) {
    m_Links.erase(std::remove_if(m_Links.begin(), m_Links.end(),
        [linkId](const Scripting::Link& l) { return l.Id == linkId; }), m_Links.end());
}

void ScriptEditorPanel::executeGraph() {
    for (auto& node : m_Nodes) {
        if (node.Type == "EventBeginPlay") {
            // Находим Exec выходной пин
            for (auto& pin : node.OutputPins) {
                if (pin.Type == Scripting::PinType::Flow) {
                    // Выполняем связанные ноды
                    for (auto& link : m_Links) {
                        if (link.StartNodeId == node.Id && link.StartPinId == pin.Id) {
                            for (auto& next : m_Nodes) {
                                if (next.Id == link.EndNodeId) {
                                    if (next.Type == "Print") {
                                        std::cout << "Print: " << m_Scene.getEntityName(m_CurrentEntity) << std::endl;
                                    }
                                    else if (next.Type == "SetPosition") {
                                        m_Scene.setPosition(m_CurrentEntity, { 200, 200 });
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void ScriptEditorPanel::saveGraph() {
    std::stringstream ss;
    ss << "{";
    ss << "\"nodes\":[";
    for (size_t i = 0; i < m_Nodes.size(); i++) {
        if (i > 0) ss << ",";
        ss << "{\"id\":" << m_Nodes[i].Id << ",\"type\":\"" << m_Nodes[i].Type
            << "\",\"x\":" << m_Nodes[i].PosX << ",\"y\":" << m_Nodes[i].PosY << "}";
    }
    ss << "],\"links\":[";
    for (size_t i = 0; i < m_Links.size(); i++) {
        if (i > 0) ss << ",";
        ss << "{\"id\":" << m_Links[i].Id << ",\"sn\":" << m_Links[i].StartNodeId
            << ",\"sp\":" << m_Links[i].StartPinId << ",\"en\":" << m_Links[i].EndNodeId
            << ",\"ep\":" << m_Links[i].EndPinId << "}";
    }
    ss << "]}";
    m_Scene.scripts[m_CurrentEntity].scriptPath = ss.str();
    std::cout << "Graph saved" << std::endl;
}

void ScriptEditorPanel::loadGraph() {
    std::string data = m_Scene.scripts[m_CurrentEntity].scriptPath;
    if (data.empty()) return;
    std::cout << "Graph loaded" << std::endl;
}

ImVec2 ScriptEditorPanel::screenToCanvas(const ImVec2& screenPos) const {
    return ImVec2(
        (screenPos.x - m_CanvasOrigin.x - m_CanvasOffset.x) / m_CanvasScale,
        (screenPos.y - m_CanvasOrigin.y - m_CanvasOffset.y) / m_CanvasScale
    );
}

ImVec2 ScriptEditorPanel::canvasToScreen(const ImVec2& canvasPos) const {
    return ImVec2(
        m_CanvasOrigin.x + canvasPos.x * m_CanvasScale + m_CanvasOffset.x,
        m_CanvasOrigin.y + canvasPos.y * m_CanvasScale + m_CanvasOffset.y
    );
}

ImU32 ScriptEditorPanel::getPinColor(Scripting::PinType type) const {
    switch (type) {
    case Scripting::PinType::Flow: return IM_COL32(255, 255, 255, 255);
    case Scripting::PinType::Float:
    case Scripting::PinType::Int: return IM_COL32(100, 200, 100, 255);
    case Scripting::PinType::Bool: return IM_COL32(255, 100, 100, 255);
    case Scripting::PinType::String: return IM_COL32(200, 150, 50, 255);
    case Scripting::PinType::Vector2: return IM_COL32(200, 200, 50, 255);
    case Scripting::PinType::Entity: return IM_COL32(100, 150, 255, 255);
    default: return IM_COL32(150, 150, 150, 255);
    }
}

const char* ScriptEditorPanel::getPinTypeName(Scripting::PinType type) const {
    switch (type) {
    case Scripting::PinType::Flow: return "Flow";
    case Scripting::PinType::Float: return "Float";
    case Scripting::PinType::Int: return "Int";
    case Scripting::PinType::Bool: return "Bool";
    case Scripting::PinType::String: return "String";
    case Scripting::PinType::Vector2: return "Vector2";
    case Scripting::PinType::Entity: return "Entity";
    default: return "Any";
    }
}