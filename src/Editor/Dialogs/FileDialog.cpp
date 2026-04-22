#include "FileDialog.hpp"
#include <iostream>
#include <algorithm>
#include <windows.h>  // Для GetLogicalDrives

FileDialog::FileDialog() {
    // Устанавливаем начальную директорию
    m_CurrentDirectory = fs::current_path().string();
    refreshDirectory();
}

void FileDialog::open(Mode mode, const std::string& title, const std::string& defaultPath) {
    m_IsOpen = true;
    m_Mode = mode;
    m_Title = title;

    fs::path path(defaultPath);
    if (fs::exists(path) && fs::is_directory(path)) {
        m_CurrentDirectory = path.string();
    }
    else if (path.has_parent_path() && fs::exists(path.parent_path())) {
        m_CurrentDirectory = path.parent_path().string();
        strcpy_s(m_FileNameBuffer, path.filename().string().c_str());
    }
    else {
        m_CurrentDirectory = fs::current_path().string();
    }

    refreshDirectory();
    strcpy_s(m_SearchBuffer, ".scene");
    m_SelectedPath.clear();

    ImGui::OpenPopup(m_Title.c_str());
}

void FileDialog::refreshDirectory() {
    m_Entries.clear();

    try {
        for (const auto& entry : fs::directory_iterator(m_CurrentDirectory)) {
            m_Entries.push_back(entry);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error reading directory: " << e.what() << std::endl;
    }

    // Сортируем: сначала папки, потом файлы
    std::sort(m_Entries.begin(), m_Entries.end(),
        [](const fs::directory_entry& a, const fs::directory_entry& b) {
            if (a.is_directory() != b.is_directory()) {
                return a.is_directory() > b.is_directory();
            }
            return a.path().filename() < b.path().filename();
        });

    // Получаем список дисков (только для Windows)
    m_Drives.clear();
    DWORD drives = GetLogicalDrives();
    for (char drive = 'A'; drive <= 'Z'; ++drive) {
        if (drives & (1 << (drive - 'A'))) {
            m_Drives.push_back(std::string(1, drive) + ":\\");
        }
    }
}

void FileDialog::navigateUp() {
    fs::path current(m_CurrentDirectory);
    if (current.has_parent_path()) {
        navigateTo(current.parent_path().string());
    }
}

void FileDialog::navigateTo(const std::string& path) {
    if (fs::exists(path) && fs::is_directory(path)) {
        m_CurrentDirectory = path;
        refreshDirectory();
    }
}

void FileDialog::render() {
    if (!m_IsOpen) return;

    // Открываем попап только в render
    ImGui::OpenPopup(m_Title.c_str());

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(700, 450), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal(m_Title.c_str(), &m_IsOpen, ImGuiWindowFlags_NoResize)) {
        renderNavigationBar();

        ImGui::Separator();

        // Основная область
        ImGui::Columns(2, "FileDialogColumns", false);

        // Левая панель - быстрые ссылки
        ImGui::BeginChild("QuickAccess", ImVec2(0, 300), true);
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Quick Access");

        // Исправлено: используем string()
        char* userProfile = nullptr;
        size_t len = 0;
        _dupenv_s(&userProfile, &len, "USERPROFILE");
        if (userProfile) {
            std::string desktopPath = std::string(userProfile) + "\\Desktop";
            if (ImGui::Selectable("Desktop")) {
                navigateTo(desktopPath);
            }

            std::string docsPath = std::string(userProfile) + "\\Documents";
            if (ImGui::Selectable("Documents")) {
                navigateTo(docsPath);
            }
            free(userProfile);
        }

        if (ImGui::Selectable("Scenes")) {
            navigateTo("./scenes");
        }
        if (ImGui::Selectable("Assets")) {
            navigateTo("./assets");
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Drives");
        renderDrives();

        ImGui::EndChild();

        ImGui::NextColumn();

        // Правая панель - список файлов
        renderFileList();

        ImGui::Columns(1);

        ImGui::Separator();

        // Нижняя панель
        if (m_Mode == Mode::Save) {
            ImGui::Text("File name:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(300);
            ImGui::InputText("##FileName", m_FileNameBuffer, sizeof(m_FileNameBuffer));
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), ".scene");
        }

        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 220);

        const char* buttonText = (m_Mode == Mode::Open) ? "Open" : "Save";
        bool canApply = (m_Mode == Mode::Open) ? !m_SelectedPath.empty() : (strlen(m_FileNameBuffer) > 0);

        if (!canApply) ImGui::BeginDisabled();
        if (ImGui::Button(buttonText, ImVec2(100, 0)) && canApply) {
            applySelection();
        }
        if (!canApply) ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 0))) {
            m_IsOpen = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void FileDialog::renderNavigationBar() {
    // Кнопка вверх
    if (ImGui::Button("^")) {
        navigateUp();
    }
    ImGui::SameLine();

    // Путь
    ImGui::Text("%s", m_CurrentDirectory.c_str());

    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    ImGui::InputText("##Search", m_SearchBuffer, sizeof(m_SearchBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        refreshDirectory();
    }
}

void FileDialog::renderFileList() {
    ImGui::BeginChild("FileList", ImVec2(0, -50), true);

    for (const auto& entry : m_Entries) {
        std::string name = entry.path().filename().string();
        bool isDir = entry.is_directory();
        bool isScene = !isDir && entry.path().extension() == ".scene";

        if (!isDir && m_Mode == Mode::Open && !isScene) continue;

        std::string icon = isDir ? "[DIR] " : "[FILE] ";
        std::string displayName = icon + name;

        ImGuiSelectableFlags flags = ImGuiSelectableFlags_AllowDoubleClick;
        bool selected = (m_SelectedPath == entry.path().string());

        if (ImGui::Selectable(displayName.c_str(), selected, flags)) {
            if (isDir) {
                if (ImGui::IsMouseDoubleClicked(0)) {
                    navigateTo(entry.path().string());
                }
                m_SelectedPath.clear();
            }
            else {
                m_SelectedPath = entry.path().string();
                if (m_Mode == Mode::Save) {
                    strcpy_s(m_FileNameBuffer, name.c_str());
                }

                if (ImGui::IsMouseDoubleClicked(0) && m_Mode == Mode::Open) {
                    applySelection();
                }
            }
        }

        if (ImGui::IsItemHovered() && !isDir) {
            ImGui::SetTooltip("%s", entry.path().string().c_str());
        }
    }

    ImGui::EndChild();
}

void FileDialog::renderDrives() {
    for (const auto& drive : m_Drives) {
        if (ImGui::Selectable(drive.c_str())) {
            navigateTo(drive);
        }
    }
}

void FileDialog::applySelection() {
    std::string finalPath;

    if (m_Mode == Mode::Open) {
        finalPath = m_SelectedPath;
    }
    else {
        finalPath = (fs::path(m_CurrentDirectory) / m_FileNameBuffer).string();
        if (finalPath.find(".scene") == std::string::npos) {
            finalPath += ".scene";
        }
    }

    m_SelectedPath = finalPath;
    m_IsOpen = false;
    ImGui::CloseCurrentPopup();

    if (m_OnFileSelected) {
        m_OnFileSelected(finalPath);
    }
}

bool FileDialog::isValidSceneFile(const std::string& path) const {
    return path.find(".scene") != std::string::npos;
}