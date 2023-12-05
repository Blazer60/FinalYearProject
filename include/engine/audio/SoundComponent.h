/**
 * @file SoundComponent.h
 * @author Ryan Purse
 * @date 05/12/2023
 */


#pragma once

#include "AudioSource.h"
#include "Component.h"
#include "Pch.h"

namespace engine
{

/**
 * @author Ryan Purse
 * @date 05/12/2023
 */
class SoundComponent
    : public Component
{
public:
    SoundComponent();
    explicit SoundComponent(const std::filesystem::path &path);
    void onUpdate() override;
    void onDrawUi() override;

    void playSound() const;

protected:
    std::unique_ptr<AudioSource> mAudioSource;
    float mVolume { 100.f };

    ENGINE_SERIALIZABLE_COMPONENT(SoundComponent);
};

} // engine
