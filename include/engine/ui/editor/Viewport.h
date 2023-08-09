/**
 * @file Viewport.h
 * @author Ryan Purse
 * @date 08/08/2023
 */


#pragma once

#include "Pch.h"
#include "Drawable.h"
#include "ImGuizmo.h"

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
        void init();
        ~Viewport() override;
        
        [[nodiscard]] glm::vec2 getSize() const;
        
    protected:
        void onDrawUi() override;
        
        glm::vec2 mSize;
        bool mIsFocused { false };
        ImGuizmo::OPERATION mOperation { ImGuizmo::OPERATION::TRANSLATE };
        uint32_t mKeyToken { 0 };
    };
    
} // engine
