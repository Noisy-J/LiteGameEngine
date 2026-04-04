#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>

class ResourceManager {
public:
	static sf::Texture& getTexture(std::string const& filename);

private:
	std::map<std::string, sf::Texture> m_Textures;

	static ResourceManager& getInstance();
	ResourceManager() {}
};