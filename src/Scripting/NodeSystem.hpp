#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <any>
#include <SFML/Graphics.hpp>

namespace Scripting {

    enum class PinType {
        Flow,
        Float,
        Int,
        Bool,
        String,
        Vector2,
        Entity,
        Any
    };

    struct Pin {
        std::string Name;
        PinType Type;
        std::string DefaultValue;
        bool Connected = false;
        int Id = -1;
    };

    struct Node {
        std::string Name;
        std::string Type;
        std::vector<Pin> InputPins;
        std::vector<Pin> OutputPins;
        float PosX = 0, PosY = 0;
        int Id = -1;
    };

    struct Link {
        int Id = -1;
        int StartNodeId;
        int StartPinId;
        int EndNodeId;
        int EndPinId;
    };

    class NodeRegistry {
    public:
        static NodeRegistry& Instance() {
            static NodeRegistry instance;
            return instance;
        }

        Node CreateNode(const std::string& type) {
            Node node;
            node.Type = type;
            int pinId = 0;

            if (type == "EventBeginPlay") {
                node.Name = "Begin Play";
                node.OutputPins.push_back({ "Exec", PinType::Flow, "", false, pinId++ });
            }
            else if (type == "EventTick") {
                node.Name = "Tick";
                node.OutputPins.push_back({ "Exec", PinType::Flow, "", false, pinId++ });
                node.OutputPins.push_back({ "DeltaTime", PinType::Float, "", false, pinId++ });
            }
            else if (type == "GetPosition") {
                node.Name = "Get Position";
                node.InputPins.push_back({ "Exec", PinType::Flow, "", false, pinId++ });
                node.InputPins.push_back({ "Target", PinType::Entity, "", false, pinId++ });
                node.OutputPins.push_back({ "Exec", PinType::Flow, "", false, pinId++ });
                node.OutputPins.push_back({ "Position", PinType::Vector2, "", false, pinId++ });
            }
            else if (type == "SetPosition") {
                node.Name = "Set Position";
                node.InputPins.push_back({ "Exec", PinType::Flow, "", false, pinId++ });
                node.InputPins.push_back({ "Target", PinType::Entity, "", false, pinId++ });
                node.InputPins.push_back({ "Position", PinType::Vector2, "", false, pinId++ });
                node.OutputPins.push_back({ "Exec", PinType::Flow, "", false, pinId++ });
            }
            else if (type == "Print") {
                node.Name = "Print String";
                node.InputPins.push_back({ "Exec", PinType::Flow, "", false, pinId++ });
                node.InputPins.push_back({ "String", PinType::String, "Hello", false, pinId++ });
                node.OutputPins.push_back({ "Exec", PinType::Flow, "", false, pinId++ });
            }
            else if (type == "Branch") {
                node.Name = "Branch";
                node.InputPins.push_back({ "Exec", PinType::Flow, "", false, pinId++ });
                node.InputPins.push_back({ "Condition", PinType::Bool, "", false, pinId++ });
                node.OutputPins.push_back({ "True", PinType::Flow, "", false, pinId++ });
                node.OutputPins.push_back({ "False", PinType::Flow, "", false, pinId++ });
            }
            else if (type == "MakeVector") {
                node.Name = "Make Vector2";
                node.InputPins.push_back({ "X", PinType::Float, "0", false, pinId++ });
                node.InputPins.push_back({ "Y", PinType::Float, "0", false, pinId++ });
                node.OutputPins.push_back({ "Vector", PinType::Vector2, "", false, pinId++ });
            }
            else if (type == "BreakVector") {
                node.Name = "Break Vector2";
                node.InputPins.push_back({ "Vector", PinType::Vector2, "", false, pinId++ });
                node.OutputPins.push_back({ "X", PinType::Float, "", false, pinId++ });
                node.OutputPins.push_back({ "Y", PinType::Float, "", false, pinId++ });
            }
            else if (type == "AddFloat") {
                node.Name = "Add (Float)";
                node.InputPins.push_back({ "A", PinType::Float, "0", false, pinId++ });
                node.InputPins.push_back({ "B", PinType::Float, "0", false, pinId++ });
                node.OutputPins.push_back({ "Result", PinType::Float, "", false, pinId++ });
            }
            else if (type == "Delay") {
                node.Name = "Delay";
                node.InputPins.push_back({ "Exec", PinType::Flow, "", false, pinId++ });
                node.InputPins.push_back({ "Duration", PinType::Float, "1.0", false, pinId++ });
                node.OutputPins.push_back({ "Completed", PinType::Flow, "", false, pinId++ });
            }
            else if (type == "GetOwner") {
                node.Name = "Get Owner";
                node.OutputPins.push_back({ "Owner", PinType::Entity, "", false, pinId++ });
            }

            return node;
        }

        std::vector<std::pair<std::string, std::string>> GetAvailableNodes() {
            return {
                {"EventBeginPlay", "Events/Begin Play"},
                {"EventTick", "Events/Tick"},
                {"GetPosition", "Transform/Get Position"},
                {"SetPosition", "Transform/Set Position"},
                {"MakeVector", "Vector/Make Vector2"},
                {"BreakVector", "Vector/Break Vector2"},
                {"AddFloat", "Math/Add (Float)"},
                {"Branch", "Logic/Branch"},
                {"Delay", "Flow/Delay"},
                {"Print", "Debug/Print String"},
                {"GetOwner", "Entity/Get Owner"},
            };
        }
    };

} // namespace Scripting