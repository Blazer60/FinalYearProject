/**
 * @file ResourceFolder.cpp
 * @author Ryan Purse
 * @date 25/10/2023
 */


#include "ResourceFolder.h"

#include "Core.h"
#include "Editor.h"
#include "EngineState.h"
#include "FileLoader.h"
#include "Loader.h"
#include "Ui.h"

namespace engine
{
    const char *const resourceImagePayload = "ResourceImagePayload";
    const char *const resourceModelPayload = "ResourceModelPayload";
    const char *const resourceScenePayload = "ResourceScenePayload";
    const char *const resourceSoundPayload = "ResourceSoundPayload";
    const char *const resourceMaterialLayerPayload = "ResourceMaterialLayerPayload";
    const char *const resourceMaterialPayload = "ResourcematerialPayload";

    ResourceFolder::ResourceFolder()
    {
        changeContentsFolder(mSelectedFolder);
    }

    void ResourceFolder::onDrawUi()
    {
        if (!isShowing)
            return;

        ImGui::Begin("Resource Folder", &isShowing);
        if (ImGui::BeginTable("ResourcesTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn("First", ImGuiTableColumnFlags_WidthStretch, 0.5f);
            ImGui::TableNextColumn();
            ImGui::BeginChild("AllFilesExplorer", ImVec2(-1, ImGui::GetContentRegionAvail().y - 10.f));
            drawDirectory(file::resourcePath());
            ImGui::EndChild();
            ImGui::TableNextColumn();
            ImGui::BeginChild("SingleFileExplorer", ImVec2(-1, ImGui::GetContentRegionAvail().y - 10.f), false, ImGuiWindowFlags_MenuBar);
            drawContents();
            ImGui::EndChild();
            ImGui::EndTable();
        }
        ImGui::End();
    }

    void ResourceFolder::drawDragDropSource(const std::filesystem::path& path, const std::string&name)
    {
        if (ImGui::BeginDragDropSource())
        {
            mDragDropPath = path;
            if (file::hasImageExtension(mDragDropPath))
                ImGui::SetDragDropPayload(resourceImagePayload, &mDragDropPath, sizeof(mDragDropPath));
            else if (file::hasModelExtension(mDragDropPath))
                ImGui::SetDragDropPayload(resourceModelPayload, &mDragDropPath, sizeof(mDragDropPath));
            else if (file::hasSceneExtension(mDragDropPath))
                ImGui::SetDragDropPayload(resourceScenePayload, &mDragDropPath, sizeof(mDragDropPath));
            else if (file::hasSoundExtension(mDragDropPath))
                ImGui::SetDragDropPayload(resourceSoundPayload, &mDragDropPath, sizeof(mDragDropPath));
            else if (file::hasMaterialLayerExtension(mDragDropPath))
                ImGui::SetDragDropPayload(resourceMaterialLayerPayload, &mDragDropPath, sizeof(mDragDropPath));
            else if (file::hasMaterialExtension(mDragDropPath))
                ImGui::SetDragDropPayload(resourceMaterialPayload, &mDragDropPath, sizeof(mDragDropPath));

            ImGui::Text("%s", name.c_str());
            ImGui::EndDragDropSource();
        }
    }

    void ResourceFolder::drawDirectory(const std::filesystem::path &path)
    {
        const std::string pathName = path.filename().string();
        bool isDirectory = status(path).type() == std::filesystem::file_type::directory;
        
        if (isDirectory)
        {
            auto flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
            if (mSelectedFolder == path)
                flags |= ImGuiTreeNodeFlags_Selected;
            const bool isOpen = ImGui::TreeNodeEx(pathName.c_str(), flags);
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                changeContentsFolder(path);
            if (isOpen)
            {
                std::vector<std::filesystem::directory_entry> fileEntries;
                for (const auto &item : std::filesystem::directory_iterator(path))
                    fileEntries.push_back(item);

                std::stable_sort(fileEntries.begin(), fileEntries.end(),
                    [](const std::filesystem::directory_entry &lhs, const std::filesystem::directory_entry &rhs) -> bool
                    {
                        return static_cast<int>(status(lhs).type()) > static_cast<int>(status(rhs).type());
                    }
                );

                for (const std::filesystem::directory_entry &item : fileEntries)
                    drawDirectory(item.path());
                
                ImGui::TreePop();
            }
        }
        else
        {
            if (ImGui::Selectable(pathName.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(0))
                    userSelectAction(path);
            }
            drawDragDropSource(path, pathName);
        }
    }

    void ResourceFolder::drawMaterialLayerModal(bool toggleMaterialLayerPopup)
    {
        if (toggleMaterialLayerPopup)
        {
            ImGui::OpenPopup("Create Material Layer");
            mNewFileName = "";
        }

        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Create Material Layer", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (toggleMaterialLayerPopup)
                ImGui::SetKeyboardFocusHere();
            if (ui::inputText("File Name", &mNewFileName, ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("Create"))
            {
                editor->setUberLayer(load::materialLayer(mSelectedFolder / format::string("%.mlpcy", mNewFileName)));
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }

    void ResourceFolder::drawMaterialModal(bool toggleMaterialPopup)
    {
        if (toggleMaterialPopup)
        {
            ImGui::OpenPopup("Create Material");
            mNewFileName = "";
        }

        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Create Material", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (toggleMaterialPopup)
                ImGui::SetKeyboardFocusHere();
            if (ui::inputText("File Name", &mNewFileName, ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("Create"))
            {
                editor->setUberMaterial(load::material(mSelectedFolder / format::string("%.mpcy", mNewFileName)));
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

    }

    void ResourceFolder::drawContents()
    {
        bool toggleMaterialLayerPopup = false;
        bool toggleMaterialPopup = false;
        if (ImGui::BeginMenuBar())
        {
            if (mSelectedFolder != file::resourcePath() && ui::backButton("Go Back a file"))
                changeContentsFolder(mSelectedFolder.parent_path());

            ImGui::SetNextItemWidth(100.f);
            ImGui::SliderFloat("##Size", &mItemSize, 32, 256);
            if (ImGui::BeginMenu("Create"))
            {
                if (ImGui::MenuItem("Material"))
                {
                    toggleMaterialPopup = true;
                }
                if (ImGui::MenuItem("Material Layer"))
                {
                    toggleMaterialLayerPopup = true;
                }

                ImGui::EndMenu();
            }

            const auto folderName = format::string("%", file::makeRelativeToResourcePath(mSelectedFolder));
            const float width = ImGui::CalcTextSize(folderName.c_str()).x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - width - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
            ImGui::Text(folderName.c_str());
            ImGui::EndMenuBar();
        }


        drawMaterialLayerModal(toggleMaterialLayerPopup);
        drawMaterialModal(toggleMaterialPopup);

        std::vector<std::filesystem::directory_entry> fileEntries;
        for (const auto &item : std::filesystem::directory_iterator(mSelectedFolder))
            fileEntries.push_back(item);

        std::stable_sort(fileEntries.begin(), fileEntries.end(),
            [](const std::filesystem::directory_entry &lhs, const std::filesystem::directory_entry &rhs) -> bool
            {
                return static_cast<int>(status(lhs).type()) > static_cast<int>(status(rhs).type());
            }
        );

        const float width = ImGui::GetContentRegionAvail().x;
        auto &style = ImGui::GetStyle();
        const float itemWidth = mItemSize + style.CellPadding.x + style.ItemSpacing.x;
        const int maxItemsPerRow = glm::max(1, static_cast<int>(glm::floor(width / itemWidth) - 1));
        const bool spanFullWidth = fileEntries.size() > maxItemsPerRow;

        const int tableFlags = spanFullWidth ? ImGuiTableFlags_SizingStretchSame : ImGuiTableFlags_None;
        if (ImGui::BeginTable("Contents Folder Table", maxItemsPerRow, tableFlags))
        {
            if (!spanFullWidth)
            {
                for (int i = 0; i < maxItemsPerRow; ++i)
                {
                    const std::string columnId = format::string("Column %", i);
                    ImGui::TableSetupColumn(columnId.c_str(), ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, itemWidth);
                }
            }

            for (const std::filesystem::directory_entry &item : fileEntries)
            {
                if (ImGui::TableNextColumn())
                {
                    const float posBefore = ImGui::GetCursorPos().y;
                    drawContentItem(item);
                    const float posAfter = ImGui::GetCursorPos().y;
                    const float height = posAfter - posBefore;
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - height);

                    const std::string cellId = format::string("Cell%", item.path());
                    ImGui::InvisibleButton(cellId.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, height));
                    drawDragDropSource(item.path(), item.path().filename().string());
                    if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemActive())
                        userSelectAction(item.path());
                }
            }

            ImGui::EndTable();
        }
    }

    void ResourceFolder::drawContentItem(const std::filesystem::directory_entry& item)
    {
        const bool isDirectory = status(item).type() == std::filesystem::file_type::directory;

        const std::string name = item.path().filename().string();

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

        if (isDirectory)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - mItemSize) / 2.f);
            ui::image(mFolderIconTexture->id(), glm::vec2(mItemSize));
        }
        else if (file::hasImageExtension(item.path()) && mTextureIcons.count(name) > 0)
        {
            const auto &image = mTextureIcons.at(name);
            const glm::vec2 size = ui::scaleImage(image->size(), glm::vec2(mItemSize));

            if (size.y < mItemSize)
                ImGui::Dummy(ImVec2(0, mItemSize - size.y));

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - size.x) / 2.f);
            ui::image(image->id(), size);
        }
        else if (file::hasMaterialExtension(item.path()))
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - mItemSize) / 2.f);
            ui::image(mMaterialIconTexture->id(), glm::vec2(mItemSize));
        }
        else if (file::hasMaterialLayerExtension(item.path()))
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - mItemSize) / 2.f);
            ui::image(mLayerIconTexture->id(), glm::vec2(mItemSize));
        }
        else
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - mItemSize) / 2.f);
            ui::image(mUnknownIconTexture->id(), glm::vec2(mItemSize));
        }

        ImGui::PopStyleVar(2);

        const float textWidth = ImGui::CalcTextSize(name.c_str()).x;
        ImGui::SetCursorPosX(glm::max(ImGui::GetCursorPosX(), ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - textWidth) / 2.f));
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
        ImGui::Text(name.c_str());
        ImGui::PopTextWrapPos();
    }

    void ResourceFolder::userSelectAction(const std::filesystem::path& path)
    {
        if (const std::filesystem::directory_entry entry(path); is_directory(entry))
            changeContentsFolder(path);
        else
        {
            if (file::hasMaterialExtension(path))
                editor->setUberMaterial(load::material(path));
            if (file::hasMaterialLayerExtension(path))
                editor->setUberLayer(load::materialLayer(path));
            if (file::hasSceneExtension(path))
                core->setScene(load::scene(path), path);
        }
    }

    void ResourceFolder::changeContentsFolder(const std::filesystem::path& path)
    {
        mSelectedFolder = path;

        mTextureIcons.clear();
        for (const auto &item : std::filesystem::directory_iterator(mSelectedFolder))
        {
            if (file::hasImageExtension(item))
                mTextureIcons[item.path().filename().string()] = load::texture(item.path());
        }
    }
}
