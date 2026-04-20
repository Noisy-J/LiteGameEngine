#include "FileUtils.hpp"
#include <filesystem>

std::string FileUtils::getFileName(const std::string& path) {
    return std::filesystem::path(path).filename().string();
}

std::string FileUtils::getFileExtension(const std::string& path) {
    return std::filesystem::path(path).extension().string();
}

std::string FileUtils::getDirectory(const std::string& path) {
    return std::filesystem::path(path).parent_path().string();
}

bool FileUtils::fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

std::vector<std::string> FileUtils::listFiles(const std::string& directory,
    const std::string& extension) {
    std::vector<std::string> files;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                if (extension.empty() || entry.path().extension() == extension) {
                    files.push_back(entry.path().string());
                }
            }
        }
    }
    catch (...) {
        // Обработка ошибок
    }

    return files;
}