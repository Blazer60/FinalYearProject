/**
 * @file ResourceFolder.cpp
 * @author Ryan Purse
 * @date 25/10/2023
 */


#include "ResourceFolder.h"
#include "FileLoader.h"

namespace engine
{
    const char *resourceImagePayload = "ResourceImagePayload";
    const char *resourceModelPayload = "ResourceModelPayload";
    const char *resourceScenePayload = "ResourceScenePayload";
    const char *resourceSoundPayload = "ResourceSoundPayload";

    void ResourceFolder::onDrawUi()
    {
        if (ImGui::Begin("Resource Folder"))
            drawDirectory(file::resourcePath());
        
        ImGui::End();
    }
    
    void ResourceFolder::drawDirectory(const std::filesystem::path &path)
    {
        const std::string pathName = path.filename().string();
        bool isDirectory = status(path).type() == std::filesystem::file_type::directory;
        
        if (isDirectory)
        {
            if (ImGui::TreeNodeEx(pathName.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
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
                
                ImGui::Text("%s", pathName.c_str());
                ImGui::EndDragDropSource();
            }
        }
    }
}
