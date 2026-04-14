#pragma once
#include <SFML/Graphics.hpp>

#include <iostream>
#include <map>
#include <string>


class ResourceManager {
public:
	static sf::Texture loadTexture(std::string const filename);
private:

};