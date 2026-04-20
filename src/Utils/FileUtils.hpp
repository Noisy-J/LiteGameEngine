#pragma once

#include <string>
#include <vector>

class FileUtils {
public:
    static std::string getFileName(const std::string& path);
    static std::string getFileExtension(const std::string& path);
    static std::string getDirectory(const std::string& path);
    static bool fileExists(const std::string& path);
    static std::vector<std::string> listFiles(const std::string& directory,
        const std::string& extension = "");
};