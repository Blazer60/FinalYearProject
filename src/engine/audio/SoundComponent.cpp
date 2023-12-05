/**
 * @file SoundComponent.cpp
 * @author Ryan Purse
 * @date 05/12/2023
 */


#include "SoundComponent.h"

#include "Actor.h"
#include "Audio.h"
#include "FileExplorer.h"
#include "ResourceFolder.h"

namespace engine
{
    SoundComponent::SoundComponent()
        : mAudioSource(std::make_unique<AudioSource>())
    {
    }

    SoundComponent::SoundComponent(const std::filesystem::path& path)
        : mAudioSource(load::audio(path))
    {

    }

    void SoundComponent::onDrawUi()
    {
        ImGui::PushID("SoundComponentSettings");
        if (ImGui::TreeNodeEx("Sound Settigns", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Destroy Componenet"))
                mActor->removeComponent(this);

            const auto path = mAudioSource->getPath().filename().string();
            ImGui::Text("Loaded sound %s", path.c_str());
            if (ImGui::Button("Load Sound"))
            {
                if (const std::string soundPath = openFileDialog(); !soundPath.empty())
                    mAudioSource = load::audio(soundPath);
            }
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(resourceSoundPayload))
                {
                    const std::filesystem::path payloadPath = *reinterpret_cast<std::filesystem::path*>(payload->Data);
                    mAudioSource = load::audio(payloadPath);
                }
            }
            if (ImGui::Button("Play Sound"))
                mAudioSource->play();

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    void SoundComponent::playSound() const
    {
        mAudioSource->play();
    }
}
