#include "ResourceManager.hpp"

sf::Texture ResourceManager::loadTexture(std::string const filename) {
    sf::Texture texture;
    ResourceManager rm;

    if (!(texture.loadFromFile(filename))) {
        std::cout << "Failed to load texture, creating test texture" << std::endl;
        texture.resize({ 100, 100 });

        // Создаем дефолт текстуру
        sf::Image image;
        image.resize({ 100, 100 }, sf::Color::White);
        for (int y = 0; y < 100; y++) {
            for (int x = 0; x < 100; x++) {
                if ((x / 10 + y / 10) % 2 == 0) {
                    image.setPixel({ (unsigned int)x, (unsigned int)y }, sf::Color::Magenta);
                }
                else {
                    image.setPixel({ (unsigned int)x, (unsigned int)y }, sf::Color::Black);
                }
            }
        }
        //Устанавливаем дефолт текстуру
        texture.update(image);
    }
    return texture;
}


/*sf::Texture ResourceManager::getTexture(std::string const filename) {
    sf::Texture texture;
    if (!(texture.loadFromFile(filename))) {
        std::cout << "Failed to load texture, creating test texture" << std::endl;
        texture.resize({ 100, 100 });

        // Создаем дефолт текстуру
        sf::Image image;
        image.resize({ 100, 100 }, sf::Color::White);
        for (int y = 0; y < 100; y++) {
            for (int x = 0; x < 100; x++) {
                if ((x / 10 + y / 10) % 2 == 0) {
                    image.setPixel({ (unsigned int)x, (unsigned int)y }, sf::Color::Magenta);
                }
                else {
                    image.setPixel({ (unsigned int)x, (unsigned int)y }, sf::Color::Black);
                }
            }
        }
        //Устанавливаем дефолт текстуру
        texture.update(image);
    }
    return texture;
}
*/
/*sf::Texture&
ResourceManager::getTexture(std::string const& filename) {

}*/