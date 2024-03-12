/**
 * @file ResourceFolder.cpp
 * @author Ryan Purse
 * @date 25/10/2023
 */


#include "ResourceFolder.h"

#include "Editor.h"
#include "EngineState.h"
#include "FileLoader.h"
#include "Ui.h"

namespace engine
{
    const char *resourceImagePayload = "ResourceImagePayload";
    const char *resourceModelPayload = "ResourceModelPayload";
    const char *resourceScenePayload = "ResourceScenePayload";
    const char *resourceSoundPayload = "ResourceSoundPayload";

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
                mSelectedFolder = path;
            if (isOpen)
            {
                for (const std::filesystem::directory_entry &item : std::filesystem::directory_iterator(path))
                    drawDirectory(item.path());
                
                ImGui::TreePop();
            }
        }
        else
        {
            if (ImGui::Selectable(pathName.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(0))
                    MESSAGE("Double Clicked! Todo!");
            }
            drawDragDropSource(path, pathName);
        }
    }

    void ResourceFolder::drawContents()
    {
        bool togglePopup = false;
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Create"))
            {
                if (ImGui::MenuItem("Material"))
                    MESSAGE("Todo!");
                if (ImGui::MenuItem("Material Layer"))
                {
                    togglePopup = true;
                    MESSAGE("Todo!");
                }

                ImGui::EndMenu();
            }

            const auto folderName = format::string("%", mSelectedFolder);
            const float width = ImGui::CalcTextSize(folderName.c_str()).x;
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - width);
            ImGui::Text(folderName.c_str());
            ImGui::EndMenuBar();
        }


        if (togglePopup)
        {
            ImGui::OpenPopup("Create Material Layer");
            mNewFileName = "";
        }

        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Create Material Layer", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (togglePopup)
                ImGui::SetKeyboardFocusHere();
            if (ui::inputText("File Name", &mNewFileName, ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("Create"))
            {
                // Todo: This needs to be through a load function and then passed to the editor.
                // Todo: Save/Load from disk: look in disk before creating a new one.
                editor->setUberLayer(std::make_shared<UberLayer>(mSelectedFolder / format::string("%.mlpcy", mNewFileName)));
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

        ImGui::SetNextItemWidth(100.f);
        ImGui::SliderFloat("Size", &mItemSize, 32, 256);
        const float width = ImGui::GetContentRegionAvail().x;
        auto &style = ImGui::GetStyle();
        const float itemWidth = mItemSize + style.CellPadding.x + style.ItemSpacing.x;
        const int maxItemsPerRow = glm::max(1, static_cast<int>(glm::floor(width / itemWidth)));

        int i = 0;
        for (const std::filesystem::directory_entry &item : std::filesystem::directory_iterator(mSelectedFolder))
        {
            auto id = format::string("##id%", i++);
            ImGui::BeginChild(id.c_str(), ImVec2(mItemSize, mItemSize), false, ImGuiWindowFlags_NoScrollbar);
            if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsWindowHovered())
            {
                const bool isDirectory = status(item).type() == std::filesystem::file_type::directory;
                if (isDirectory)
                    mSelectedFolder = item.path();
            }
            drawDragDropSource(item.path(), item.path().filename().string());
            drawContentItem(item);
            ImGui::EndChild();

            if (i % maxItemsPerRow != 0)
                ImGui::SameLine(static_cast<float>(i % maxItemsPerRow) / static_cast<float>(maxItemsPerRow) * width);
        }
    }

    void ResourceFolder::drawContentItem(const std::filesystem::directory_entry& item)
    {
        const bool isDirectory = status(item).type() == std::filesystem::file_type::directory;

        const std::string name = item.path().filename().string();
        const ImVec2 nameSize = ImGui::CalcTextSize(name.c_str());
        const float imageHeight = ImGui::GetContentRegionAvail().y - nameSize.y;

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

        ImGui::SetCursorPosX((mItemSize - imageHeight) / 2.f);
        if (isDirectory)
            ui::image(mFolderIconTexture.id(), glm::vec2(imageHeight));
        else
            ui::image(mUnknownIconTexture.id(), glm::vec2(imageHeight));

        ImGui::PopStyleVar(2);

        ImGui::SetCursorPosX(glm::max(0.f, ImGui::GetContentRegionAvail().x / 2.f - nameSize.x / 2.f));
        ImGui::Text(name.c_str());
    }
}
