/**
 * @file Viewport.h
 * @author Ryan Purse
 * @date 08/08/2023
 */


#pragma once

#include "Pch.h"
#include "Drawable.h"

namespace engine
{

/**
 * @author Ryan Purse
 * @date 08/08/2023
 */
    class Viewport
        : public ui::Drawable
    {
    public:
        [[nodiscard]] glm::vec2 getSize() const;
        
    protected:
        void onDrawUi() override;
        
        glm::vec2 mSize;
    };
    
} // engine
