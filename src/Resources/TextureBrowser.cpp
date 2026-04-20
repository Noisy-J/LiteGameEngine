#include "TextureBrowser.hpp"
#include "ResourceManager.hpp"
#include <filesystem>
#include <algorithm>
#include <iostream>

TextureBrowser::TextureBrowser(Scene& scene)
    : m_Scene(scene) {
}

//Проверка директории на наличие текстур нужных форматов
void TextureBrowser::scanAvailableTextures(const std::string& directory) {
    m_AvailableTextures.clear();

    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") {
                    m_AvailableTextures.push_back(entry.path().string());
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error scanning textures: " << e.what() << std::endl;
    }

    std::sort(m_AvailableTextures.begin(), m_AvailableTextures.end());
}

//Загрузчик текстуры из выбранного файла
void TextureBrowser::loadTextureToEntity(Entity entity, const std::string& path) {
    try {
        auto texture = ResourceManager::loadTexture(path);
        m_Scene.setTexture(entity, texture);
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load texture: " << e.what() << std::endl;
    }
}