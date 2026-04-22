#pragma once

#include <imgui.h>
#include <imgui-SFML.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <filesystem>
#include <SFML/Graphics.hpp>

namespace fs = std::filesystem;

class ContentBrowserPanel {
public:
    ContentBrowserPanel();
    ~ContentBrowserPanel();

    void render();

    void setOnAssetSelected(std::function<void(const std::string&)> callback) { m_OnAssetSelected = callback; }
    void setOnAssetDoubleClicked(std::function<void(const std::string&)> callback) { m_OnAssetDoubleClicked = callback; }

    void navigateTo(const std::string& path);
    void setRootDirectory(const std::string& path);
    void refreshDirectory();

private:
    std::string m_CurrentDirectory{ "./assets/textures" };
    std::string m_RootDirectory{ "./assets/textures" };
    std::vector<std::string> m_NavigationHistory;
    int m_HistoryIndex{ -1 };

    struct AssetInfo {
        std::string name;
        std::string path;
        std::string extension;
        bool isDirectory{ false };
        bool isTexture{ false };
        std::shared_ptr<sf::Texture> texture;
        sf::Vector2u textureSize{ 0, 0 };
    };

    std::vector<AssetInfo> m_Assets;
    std::unordered_map<std::string, std::shared_ptr<sf::Texture>> m_ThumbnailCache;

    float m_ThumbnailSize{ 80.f };
    float m_Padding{ 8.f };
    bool m_ShowFileExtensions{ true };
    int m_SelectedAssetIndex{ -1 };

    char m_SearchBuffer[128] = "";
    bool m_ShowOnlyTextures{ false };

    bool m_ShowPreview{ true };
    AssetInfo* m_PreviewAsset{ nullptr };

    // Для диалога импорта
    bool m_ShowImportDialog{ false };
    std::string m_ImportSourcePath;
    char m_ImportFileName[256] = "";

    bool m_PendingOpenPopup{ false };

    // Быстрые ссылки
    std::vector<std::pair<std::string, std::string>> m_QuickLinks;

    std::function<void(const std::string&)> m_OnAssetSelected;
    std::function<void(const std::string&)> m_OnAssetDoubleClicked;

    void renderNavigationBar();
    void renderFileGrid();
    void renderPreviewPanel();
    void renderAssetItem(AssetInfo& asset, int index);
    void renderContextMenu(AssetInfo& asset);
    void renderImportDialog();
    void renderQuickLinks();

    void navigateBack();
    void navigateForward();
    void navigateUp();
    void addToHistory(const std::string& path);

    std::shared_ptr<sf::Texture> loadThumbnail(const std::string& path);
    void clearThumbnailCache();

    bool isTextureFile(const std::string& extension) const;
    bool matchesSearch(const AssetInfo& asset) const;

    void handleDragDrop(const AssetInfo& asset);
    void handleClick(const AssetInfo& asset, int index);
    void handleDoubleClick(const AssetInfo& asset);

    std::string formatFileSize(uintmax_t size) const;

    // Новые методы для импорта
    void openImportDialog();
    bool importFile();
    bool copyFile(const fs::path& source, const fs::path& destination);
    void initQuickLinks();
};