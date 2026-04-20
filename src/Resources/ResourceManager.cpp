#include "ResourceManager.hpp"
#include <iostream>


std::unordered_map<std::string, std::shared_ptr<sf::Texture>> ResourceManager::s_Textures;//Массив текстур
std::unordered_map<std::string, std::shared_ptr<sf::Font>> ResourceManager::s_Fonts;//Массив шрифтов

//Загрузка текстур
std::shared_ptr<sf::Texture> ResourceManager::loadTexture(const std::string& path) {
    //Если текстура уже есть, возвращаем
    auto it = s_Textures.find(path);
    if (it != s_Textures.end()) {
        return it->second;
    }

    //Если нет загружаем, добавляем в массив и возвращаем
    auto texture = std::make_shared<sf::Texture>();
    if (!texture->loadFromFile(path)) {
        throw std::runtime_error("Failed to load texture: " + path);
    }

    s_Textures[path] = texture;
    return texture;
}

//Тот же процесс со шрифтами
std::shared_ptr<sf::Font> ResourceManager::loadFont(const std::string& path) {
    auto it = s_Fonts.find(path);
    if (it != s_Fonts.end()) {
        return it->second;
    }

    auto font = std::make_shared<sf::Font>();
    if (!font->openFromFile(path)) {
        throw std::runtime_error("Failed to load font: " + path);
    }

    s_Fonts[path] = font;
    return font;
}

void ResourceManager::clearCache() {
    s_Textures.clear();
    s_Fonts.clear();
}

void ResourceManager::removeTexture(const std::string& path) {
    s_Textures.erase(path);
}