#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

class GameObject{
public:


	GameObject();

	virtual ~GameObject() = default;
	virtual void update(float dt) = 0;

private:

	sf::Sprite* createObject(std::string objectrName);

	void setTexture(sf::Sprite* sprite, std::string textureName);

	void setPosition(sf::Vector2f pos);

	sf::Vector2f getPosition(sf::Sprite sprite);

};