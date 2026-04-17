#pragma once
#include "imgui.h"
#include "imgui-SFML.h"
#include <SFML/Graphics.hpp>
#include "System.hpp"
#include "ResourceManager.hpp"
#include <iostream>

class Engine {
public:
    Engine();
    void run();
    //void resetCamera();
    //void centerCameraOnEntity(Entity entity);

private:
    void input();
    void update(sf::Time dt, Scene* scene);
    void draw();
    void renderInspector(Entity entity, Scene* scene);
    void renderCreatePanel();


    sf::RenderWindow m_Window;
    sf::Clock m_Clock;
    Scene m_Scene;
    sf::Texture m_Texture;
    sf::RenderTexture m_Viewport;
    Entity m_Player;

    // Camera dragging
    bool m_isDraggingCamera;
    sf::Vector2i m_lastMousePos;

    // Entity dragging
    bool m_isDraggingEntity;
    Entity m_SelectedEntity;
    sf::Vector2f m_dragOffset;
    sf::Vector2f m_dragStartPos;
    float m_dragThreshold;


    sf::Vector2f m_viewportPos;  // Позиция вьюпорта в окне
    sf::Vector2f m_viewportSize; // Размер вьюпорта
    sf::Vector2f m_viewportSizeWindow; // Размер окна вьюпорта
    float m_zoomX;
    float m_zoomY;
};