#include "ContentBrowserPanel.hpp"
#include "Resources/ResourceManager.hpp"
#include <iostream>
#include <algorithm>
#include <windows.h>
#include <shlobj.h>
#include <commdlg.h>

ContentBrowserPanel::ContentBrowserPanel() {
    // Используем абсолютный путь
    m_CurrentDirectory = fs::absolute("./assets/textures").string();
    if (!fs::exists(m_CurrentDirectory)) {
        fs::create_directories(m_CurrentDirectory);
    }
    m_RootDirectory = m_CurrentDirectory;
    initQuickLinks();
    refreshDirectory();
    addToHistory(m_CurrentDirectory);

    std::cout << "Content Browser initialized at: " << m_CurrentDirectory << std::endl;
}

ContentBrowserPanel::~ContentBrowserPanel() {
    clearThumbnailCache();
}

void ContentBrowserPanel::initQuickLinks() {
    m_QuickLinks.clear();

    char desktopPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, 0, desktopPath))) {
        m_QuickLinks.push_back({ "Desktop", std::string(desktopPath) });
    }

    char docsPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, docsPath))) {
        m_QuickLinks.push_back({ "Documents", std::string(docsPath) });
    }

    char downloadsPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, downloadsPath))) {
        std::string downloads = std::string(downloadsPath) + "\\Downloads";
        if (fs::exists(downloads)) {
            m_QuickLinks.push_back({ "Downloads", downloads });
        }
    }

    char picturesPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_MYPICTURES, NULL, 0, picturesPath))) {
        m_QuickLinks.push_back({ "Pictures", std::string(picturesPath) });
    }

    // Используем абсолютные пути
    m_QuickLinks.push_back({ "Project Assets", fs::absolute("./assets").string() });
    m_QuickLinks.push_back({ "Textures", fs::absolute("./assets/textures").string() });
    m_QuickLinks.push_back({ "Scenes", fs::absolute("./scenes").string() });

    DWORD drives = GetLogicalDrives();
    for (char drive = 'A'; drive <= 'Z'; ++drive) {
        if (drives & (1 << (drive - 'A'))) {
            std::string drivePath = std::string(1, drive) + ":\\";
            m_QuickLinks.push_back({ drivePath, drivePath });
        }
    }
}

void ContentBrowserPanel::render() {
    ImGui::Begin("Content Browser");

    renderNavigationBar();
    ImGui::Separator();

    ImGui::Columns(2, "ContentBrowserColumns");
    ImGui::SetColumnWidth(0, 180);

    renderQuickLinks();

    ImGui::NextColumn();

    if (m_ShowPreview) {
        float previewWidth = 200;
        float fileGridWidth = ImGui::GetColumnWidth() - previewWidth;

        ImGui::BeginChild("FileGridContainer", ImVec2(fileGridWidth, 0), false);
        renderFileGrid();
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginChild("PreviewContainer", ImVec2(previewWidth, 0), false);
        renderPreviewPanel();
        ImGui::EndChild();
    }
    else {
        renderFileGrid();
    }

    ImGui::Columns(1);

    // Открываем попап здесь, в основном цикле рендеринга
    if (m_ShowImportDialog && m_PendingOpenPopup) {
        ImGui::OpenPopup("Import File");
        m_PendingOpenPopup = false;
    }

    if (m_ShowImportDialog) {
        renderImportDialog();
    }

    ImGui::End();
}

void ContentBrowserPanel::renderQuickLinks() {
    ImGui::BeginChild("QuickLinks", ImVec2(0, 0), true);

    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Quick Access");
    ImGui::Separator();

    for (const auto& link : m_QuickLinks) {
        if (ImGui::Selectable(link.first.c_str())) {
            if (fs::exists(link.second) && fs::is_directory(link.second)) {
                navigateTo(link.second);
                std::cout << "Navigated to: " << link.second << std::endl;
            }
            else {
                std::cout << "Cannot navigate to: " << link.second << " (exists: " << fs::exists(link.second) << ")" << std::endl;
            }
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", link.second.c_str());
        }
    }

    ImGui::EndChild();
}

void ContentBrowserPanel::refreshDirectory() {
    m_Assets.clear();
    m_SelectedAssetIndex = -1;
    m_PreviewAsset = nullptr;

    std::cout << "Refreshing directory: " << m_CurrentDirectory << std::endl;

    if (!fs::exists(m_CurrentDirectory)) {
        std::cout << "Directory does not exist, creating..." << std::endl;
        fs::create_directories(m_CurrentDirectory);
    }

    try {
        for (const auto& entry : fs::directory_iterator(m_CurrentDirectory)) {
            AssetInfo asset;
            asset.name = entry.path().filename().string();
            asset.path = entry.path().string();
            asset.extension = entry.path().extension().string();
            asset.isDirectory = entry.is_directory();

            if (!asset.isDirectory) {
                std::transform(asset.extension.begin(), asset.extension.end(),
                    asset.extension.begin(), ::tolower);
                asset.isTexture = isTextureFile(asset.extension);
            }

            m_Assets.push_back(asset);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error reading directory: " << e.what() << std::endl;
    }

    std::sort(m_Assets.begin(), m_Assets.end(), [](const AssetInfo& a, const AssetInfo& b) {
        if (a.isDirectory != b.isDirectory) return a.isDirectory > b.isDirectory;
        if (a.isTexture != b.isTexture) return a.isTexture > b.isTexture;
        return a.name < b.name;
        });

    std::cout << "Found " << m_Assets.size() << " items" << std::endl;
}

void ContentBrowserPanel::renderNavigationBar() {
    if (ImGui::Button("<")) {
        navigateBack();
    }
    ImGui::SameLine();
    if (ImGui::Button(">")) {
        navigateForward();
    }
    ImGui::SameLine();
    if (ImGui::Button("^")) {
        navigateUp();
    }
    ImGui::SameLine();

    ImGui::Text("%s", m_CurrentDirectory.c_str());

    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 380);
    ImGui::SetNextItemWidth(150);
    ImGui::InputTextWithHint("##Search", "Search...", m_SearchBuffer, sizeof(m_SearchBuffer));

    ImGui::SameLine();
    ImGui::Checkbox("Textures", &m_ShowOnlyTextures);
    ImGui::SameLine();
    ImGui::Checkbox("Preview", &m_ShowPreview);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::SliderFloat("##Size", &m_ThumbnailSize, 48.f, 128.f, "%.0f");

    ImGui::SameLine();
    if (ImGui::Button("Import")) {
        openImportDialog();
    }
}

void ContentBrowserPanel::renderFileGrid() {
    float contentWidth = ImGui::GetContentRegionAvail().x;
    int columns = std::max(1, static_cast<int>(contentWidth / (m_ThumbnailSize + m_Padding)));

    ImGui::BeginChild("FileGrid", ImVec2(0, 0), true);

    int index = 0;
    int renderedCount = 0;

    for (auto& asset : m_Assets) {
        if (!matchesSearch(asset)) continue;
        if (m_ShowOnlyTextures && !asset.isTexture && !asset.isDirectory) continue;

        renderAssetItem(asset, index);

        if ((renderedCount + 1) % columns != 0) {
            ImGui::SameLine();
        }

        index++;
        renderedCount++;
    }

    if (ImGui::BeginPopupContextWindow("EmptySpaceContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        if (ImGui::MenuItem("Refresh")) refreshDirectory();
        if (ImGui::MenuItem("New Folder")) {
            std::string newFolder = m_CurrentDirectory + "\\NewFolder";
            int counter = 1;
            while (fs::exists(newFolder)) {
                newFolder = m_CurrentDirectory + "\\NewFolder" + std::to_string(counter++);
            }
            if (fs::create_directory(newFolder)) {
                refreshDirectory();
            }
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Import File...")) {
            openImportDialog();
        }
        ImGui::EndPopup();
    }

    ImGui::EndChild();
}

void ContentBrowserPanel::renderAssetItem(AssetInfo& asset, int index) {
    ImGui::PushID(asset.path.c_str());
    ImGui::BeginGroup();

    ImVec2 itemSize(m_ThumbnailSize, m_ThumbnailSize);
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    bool isSelected = (m_SelectedAssetIndex == index);
    if (isSelected) {
        ImGui::GetWindowDrawList()->AddRectFilled(
            cursorPos,
            ImVec2(cursorPos.x + itemSize.x, cursorPos.y + itemSize.y),
            IM_COL32(50, 100, 200, 100)
        );
    }

    if (asset.isDirectory) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.4f, 0.5f, 1.0f));
        ImGui::Button(("##dir_" + asset.name).c_str(), itemSize);
        ImGui::PopStyleColor();

        // Текст "[DIR]" вместо смайлика
        ImVec2 textSize = ImGui::CalcTextSize("[DIR]");
        ImGui::SetCursorScreenPos(ImVec2(
            cursorPos.x + (itemSize.x - textSize.x) * 0.5f,
            cursorPos.y + (itemSize.y - textSize.y) * 0.5f
        ));
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "[DIR]");
    }
    else if (asset.isTexture) {
        if (!asset.texture) {
            asset.texture = loadThumbnail(asset.path);
            if (asset.texture) {
                asset.textureSize = asset.texture->getSize();
            }
        }

        if (asset.texture) {
            sf::Vector2f size(m_ThumbnailSize, m_ThumbnailSize);
            ImGui::Image(*asset.texture, size);
        }
        else {
            ImGui::Button(asset.name.c_str(), itemSize);
        }
    }
    else {
        ImGui::Button(asset.name.c_str(), itemSize);
    }

    if (ImGui::IsItemHovered()) {
        if (ImGui::IsMouseDoubleClicked(0)) {
            handleDoubleClick(asset);
        }
        if (ImGui::IsMouseClicked(0)) {
            handleClick(asset, index);
        }

        ImGui::BeginTooltip();
        ImGui::Text("%s", asset.name.c_str());
        if (!asset.isDirectory) {
            try {
                auto size = fs::file_size(asset.path);
                ImGui::Text("Size: %s", formatFileSize(size).c_str());
            }
            catch (...) {}
        }
        ImGui::EndTooltip();
    }

    handleDragDrop(asset);
    renderContextMenu(asset);

    std::string displayName = asset.name;
    if (!m_ShowFileExtensions && !asset.isDirectory) {
        displayName = fs::path(asset.name).stem().string();
    }
    if (displayName.length() > 12) {
        displayName = displayName.substr(0, 9) + "...";
    }

    ImVec2 textSize = ImGui::CalcTextSize(displayName.c_str());
    ImGui::SetCursorScreenPos(ImVec2(
        cursorPos.x + (itemSize.x - textSize.x) * 0.5f,
        cursorPos.y + itemSize.y + 4
    ));
    ImGui::Text("%s", displayName.c_str());

    ImGui::EndGroup();
    ImGui::PopID();
}

void ContentBrowserPanel::renderPreviewPanel() {
    ImGui::BeginChild("Preview", ImVec2(0, 0), true);

    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Preview");
    ImGui::Separator();

    if (m_PreviewAsset && m_PreviewAsset->isTexture) {
        if (!m_PreviewAsset->texture) {
            m_PreviewAsset->texture = loadThumbnail(m_PreviewAsset->path);
            if (m_PreviewAsset->texture) {
                m_PreviewAsset->textureSize = m_PreviewAsset->texture->getSize();
            }
        }

        if (m_PreviewAsset->texture) {
            float availableWidth = ImGui::GetContentRegionAvail().x;
            sf::Vector2f imageSize(availableWidth, availableWidth);

            auto& texSize = m_PreviewAsset->textureSize;
            if (texSize.x > 0 && texSize.y > 0) {
                float aspect = static_cast<float>(texSize.y) / static_cast<float>(texSize.x);
                imageSize.y = imageSize.x * aspect;
            }

            ImGui::Image(*m_PreviewAsset->texture, imageSize);
        }

        ImGui::Text("%s", m_PreviewAsset->name.c_str());

        if (m_PreviewAsset->textureSize.x > 0) {
            ImGui::Text("Size: %ux%u",
                m_PreviewAsset->textureSize.x,
                m_PreviewAsset->textureSize.y);
        }

        try {
            auto fileSize = fs::file_size(m_PreviewAsset->path);
            ImGui::Text("File: %s", formatFileSize(fileSize).c_str());
        }
        catch (...) {}
    }
    else if (m_PreviewAsset && m_PreviewAsset->isDirectory) {
        ImGui::Text("Folder: %s", m_PreviewAsset->name.c_str());
    }
    else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Select an asset to preview");
    }

    ImGui::EndChild();
}

void ContentBrowserPanel::renderContextMenu(AssetInfo& asset) {
    if (ImGui::BeginPopupContextItem((asset.name + "_context").c_str())) {
        if (asset.isDirectory) {
            if (ImGui::MenuItem("Open")) {
                navigateTo(asset.path);
            }
            ImGui::Separator();
        }

        if (ImGui::MenuItem("Delete")) {
            try {
                fs::remove_all(asset.path);
                refreshDirectory();
            }
            catch (...) {}
        }

        if (ImGui::MenuItem("Copy Path")) {
            ImGui::SetClipboardText(asset.path.c_str());
        }

        if (ImGui::MenuItem("Show in Explorer")) {
            std::string cmd = "explorer /select,\"" + asset.path + "\"";
            system(cmd.c_str());
        }

        ImGui::EndPopup();
    }
}

void ContentBrowserPanel::navigateTo(const std::string& path) {
    std::cout << "navigateTo called with: " << path << std::endl;

    if (fs::exists(path) && fs::is_directory(path)) {
        m_CurrentDirectory = path;
        addToHistory(path);
        refreshDirectory();
        std::cout << "Navigation successful" << std::endl;
    }
    else {
        std::cout << "Navigation failed: path does not exist or is not a directory" << std::endl;
    }
}

void ContentBrowserPanel::navigateBack() {
    if (m_HistoryIndex > 0) {
        m_HistoryIndex--;
        m_CurrentDirectory = m_NavigationHistory[m_HistoryIndex];
        refreshDirectory();
    }
}

void ContentBrowserPanel::navigateForward() {
    if (m_HistoryIndex < static_cast<int>(m_NavigationHistory.size()) - 1) {
        m_HistoryIndex++;
        m_CurrentDirectory = m_NavigationHistory[m_HistoryIndex];
        refreshDirectory();
    }
}

void ContentBrowserPanel::navigateUp() {
    fs::path current(m_CurrentDirectory);
    if (current.has_parent_path()) {
        navigateTo(current.parent_path().string());
    }
}

void ContentBrowserPanel::addToHistory(const std::string& path) {
    m_HistoryIndex++;
    if (m_HistoryIndex < static_cast<int>(m_NavigationHistory.size())) {
        m_NavigationHistory.erase(
            m_NavigationHistory.begin() + m_HistoryIndex,
            m_NavigationHistory.end()
        );
    }
    m_NavigationHistory.push_back(path);
}

void ContentBrowserPanel::handleDoubleClick(const AssetInfo& asset) {
    std::cout << "Double clicked: " << asset.name << " isDir: " << asset.isDirectory << std::endl;

    if (asset.isDirectory) {
        navigateTo(asset.path);
    }
    else if (m_OnAssetDoubleClicked) {
        m_OnAssetDoubleClicked(asset.path);
    }
}

// ... остальные функции (openImportDialog, renderImportDialog, copyFile, importFile, loadThumbnail, clearThumbnailCache, isTextureFile, matchesSearch, handleDragDrop, handleClick, formatFileSize, setRootDirectory) остаются без изменений ...

std::string ContentBrowserPanel::formatFileSize(uintmax_t size) const {
    const char* units[] = { "B", "KB", "MB", "GB", "TB" };
    int unitIndex = 0;
    double displaySize = static_cast<double>(size);

    while (displaySize >= 1024.0 && unitIndex < 4) {
        displaySize /= 1024.0;
        unitIndex++;
    }

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.1f %s", displaySize, units[unitIndex]);
    return std::string(buffer);
}

void ContentBrowserPanel::setRootDirectory(const std::string& path) {
    m_RootDirectory = path;
    navigateTo(path);
}

void ContentBrowserPanel::openImportDialog() {
    std::cout << "=== openImportDialog called ===" << std::endl;

    OPENFILENAMEA ofn;
    char fileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tga\0All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = "png";

    if (GetOpenFileNameA(&ofn)) {
        m_ImportSourcePath = fileName;
        fs::path sourcePath(fileName);
        strcpy_s(m_ImportFileName, sizeof(m_ImportFileName), sourcePath.filename().string().c_str());
        m_ShowImportDialog = true;
        m_PendingOpenPopup = true;  // Флаг для открытия в render

        std::cout << "File selected: " << m_ImportSourcePath << std::endl;
    }
}

void ContentBrowserPanel::renderImportDialog() {
    // Всегда открываем попап, если флаг установлен
    if (m_ShowImportDialog) {
        ImGui::OpenPopup("Import File");
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(450, 200), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Import File", &m_ShowImportDialog, ImGuiWindowFlags_NoResize)) {
        ImGui::Text("Source: %s", m_ImportSourcePath.c_str());
        ImGui::Text("Destination: %s", m_CurrentDirectory.c_str());
        ImGui::Separator();

        ImGui::Text("Save as:");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##ImportFileName", m_ImportFileName, sizeof(m_ImportFileName));

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::Button("Import", ImVec2(100, 0))) {
            if (importFile()) {
                m_ShowImportDialog = false;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 0))) {
            m_ShowImportDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

bool ContentBrowserPanel::copyFile(const fs::path& source, const fs::path& destination) {
    std::cout << "copyFile:" << std::endl;
    std::cout << "  Source: " << source.string() << std::endl;
    std::cout << "  Dest: " << destination.string() << std::endl;

    try {
        if (!fs::exists(source)) {
            std::cout << "  ERROR: Source file does not exist!" << std::endl;
            return false;
        }

        std::cout << "  Source exists, size: " << fs::file_size(source) << " bytes" << std::endl;

        if (destination.has_parent_path()) {
            fs::create_directories(destination.parent_path());
            std::cout << "  Created parent directories" << std::endl;
        }

        fs::copy_file(source, destination, fs::copy_options::overwrite_existing);
        std::cout << "  Copy successful!" << std::endl;
        return true;
    }
    catch (const fs::filesystem_error& e) {
        std::cout << "  Filesystem error: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e) {
        std::cout << "  Error: " << e.what() << std::endl;
        return false;
    }
}

bool ContentBrowserPanel::importFile() {
    std::cout << "\n=== importFile called ===" << std::endl;
    std::cout << "m_ImportSourcePath: '" << m_ImportSourcePath << "'" << std::endl;
    std::cout << "m_ImportFileName: '" << m_ImportFileName << "'" << std::endl;
    std::cout << "m_CurrentDirectory: '" << m_CurrentDirectory << "'" << std::endl;

    if (m_ImportSourcePath.empty()) {
        std::cout << "ERROR: Source path is empty!" << std::endl;
        return false;
    }

    if (strlen(m_ImportFileName) == 0) {
        std::cout << "ERROR: Filename is empty!" << std::endl;
        return false;
    }

    fs::path sourcePath(m_ImportSourcePath);
    fs::path destPath = fs::path(m_CurrentDirectory) / m_ImportFileName;

    std::cout << "Source path: " << sourcePath.string() << std::endl;
    std::cout << "Dest path: " << destPath.string() << std::endl;

    // Проверяем что sourcePath - это файл, а не папка
    if (!fs::exists(sourcePath)) {
        std::cout << "ERROR: Source file does not exist!" << std::endl;
        return false;
    }

    if (fs::is_directory(sourcePath)) {
        std::cout << "ERROR: Source is a directory, not a file!" << std::endl;
        return false;
    }

    // Если файл уже существует, создаём новое имя
    if (fs::exists(destPath)) {
        std::cout << "Destination already exists, creating new name..." << std::endl;
        std::string stem = destPath.stem().string();
        std::string ext = destPath.extension().string();
        int counter = 1;
        while (fs::exists(destPath)) {
            std::string newName = stem + "_" + std::to_string(counter++) + ext;
            destPath = fs::path(m_CurrentDirectory) / newName;
        }
        std::cout << "New dest path: " << destPath.string() << std::endl;
    }

    // Копируем файл
    std::cout << "Calling copyFile..." << std::endl;
    bool result = copyFile(sourcePath, destPath);

    if (result) {
        std::cout << "Copy successful, refreshing directory..." << std::endl;
        refreshDirectory();
        m_ThumbnailCache.erase(destPath.string());
        std::cout << "=== Import completed successfully! ===" << std::endl;
    }
    else {
        std::cout << "=== Import failed! ===" << std::endl;
    }

    return result;
}
std::shared_ptr<sf::Texture> ContentBrowserPanel::loadThumbnail(const std::string& path) {
    auto it = m_ThumbnailCache.find(path);
    if (it != m_ThumbnailCache.end()) {
        return it->second;
    }
    try {
        auto texture = ResourceManager::loadTexture(path);
        m_ThumbnailCache[path] = texture;
        return texture;
    }
    catch (...) {
        return nullptr;
    }
}

void ContentBrowserPanel::clearThumbnailCache() {
    m_ThumbnailCache.clear();
}

bool ContentBrowserPanel::isTextureFile(const std::string& extension) const {
    static const std::vector<std::string> textureExtensions = {
        ".png", ".jpg", ".jpeg", ".bmp", ".gif", ".tga", ".psd"
    };
    return std::find(textureExtensions.begin(), textureExtensions.end(), extension)
        != textureExtensions.end();
}

bool ContentBrowserPanel::matchesSearch(const AssetInfo& asset) const {
    if (strlen(m_SearchBuffer) == 0) return true;
    std::string search = m_SearchBuffer;
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);
    std::string name = asset.name;
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    return name.find(search) != std::string::npos;
}

void ContentBrowserPanel::handleDragDrop(const AssetInfo& asset) {
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        std::string payload = "ASSET:" + asset.path;
        ImGui::SetDragDropPayload("ASSET_PATH", payload.c_str(), payload.size() + 1);
        ImGui::Text("%s", asset.name.c_str());
        if (asset.isTexture && asset.texture) {
            ImGui::Image(*asset.texture, sf::Vector2f(50, 50));
        }
        ImGui::EndDragDropSource();
    }
}

void ContentBrowserPanel::handleClick(const AssetInfo& asset, int index) {
    m_SelectedAssetIndex = index;
    m_PreviewAsset = const_cast<AssetInfo*>(&asset);
    if (m_OnAssetSelected) {
        m_OnAssetSelected(asset.path);
    }
}
