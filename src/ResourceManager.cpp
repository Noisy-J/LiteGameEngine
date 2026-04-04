#include "ResourceManager.hpp"

ResourceManager&
ResourceManager::getInstance() {

	static ResourceManager instance;
	return instance;

}

sf::Texture&
ResourceManager::getTexture(std::string const& filename) {

	auto& textureMap = getInstance().m_Textures;

	auto pairFound = textureMap.find(filename);

	if (pairFound != textureMap.end()) {
		return pairFound->second;
	}
	else {
		auto& texture = textureMap[filename];
		texture.loadFromFile(filename);
		return texture;
	}
}