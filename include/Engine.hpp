#pragma once
#include "imgui.h"
#include "imgui-SFML.h"
#include <SFML/Graphics.hpp>
//#include "Manager.hpp"

class Engine {
public:
	Engine();
	void run();


private:

	void input();
	void update(sf::Time dt, sf::Sprite sprite);
	void draw(sf::Sprite sprite);

	sf::RenderWindow m_Window;
	sf::Clock m_Clock;
	//sf::Sprite m_PlayerSprite;

};