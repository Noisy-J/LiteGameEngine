#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <functional>
#include <filesystem>

namespace fs = std::filesystem;

class FileDialog {
public:
    enum class Mode {
        Open,
        Save
    };

    FileDialog();
    ~FileDialog() = default;

    void open(Mode mode, const std::string& title, const std::string& defaultPath = "./scenes");
    void render();
    bool isOpen() const { return m_IsOpen; }

    void setOnFileSelected(std::function<void(const std::string&)> callback) { m_OnFileSelected = callback; }
    std::string getSelectedPath() const { return m_SelectedPath; }

private:
    bool m_IsOpen{ false };
    Mode m_Mode{ Mode::Open };
    std::string m_Title;
    std::string m_CurrentDirectory;
    std::string m_SelectedPath;
    char m_FileNameBuffer[256] = "";
    char m_SearchBuffer[64] = "";

    std::vector<fs::directory_entry> m_Entries;
    std::vector<std::string> m_Drives;

    std::function<void(const std::string&)> m_OnFileSelected;

    void refreshDirectory();
    void navigateUp();
    void navigateTo(const std::string& path);
    void renderNavigationBar();
    void renderFileList();
    void renderDrives();
    void applySelection();

    bool isValidSceneFile(const std::string& path) const;
};