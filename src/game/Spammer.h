/**
 * @file Spammer.h
 * @author Ryan Purse
 * @date 28/10/2023
 */


#pragma once

#include "Pch.h"
#include "Component.h"


/**
 * @author Ryan Purse
 * @date 28/10/2023
 */
class Spammer
    : public engine::Component
{
public:
    ~Spammer() override;
    
protected:
    void onBegin() override;
    void onUpdate() override;
    void onDrawUi() override;
    
    std::vector<Ref<engine::Actor>> mActors;
    Ref<engine::MeshRenderer> mMeshRenderer;
};
