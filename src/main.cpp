#include "Core/Engine.hpp"
#include <iostream>

int main() {
    try {
        Engine engine;
        engine.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}