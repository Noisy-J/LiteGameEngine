#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <unordered_map>

class ResourceManager {
public:
    static std::shared_ptr<sf::Texture> loadTexture(const std::string& path);
    static std::shared_ptr<sf::Font> loadFont(const std::string& path);

    static void clearCache();
    static void removeTexture(const std::string& path);

private:
    static std::unordered_map<std::string, std::shared_ptr<sf::Texture>> s_Textures;
    static std::unordered_map<std::string, std::shared_ptr<sf::Font>> s_Fonts;
};