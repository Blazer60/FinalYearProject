/**
 * @file ProfilerViewer.h
 * @author Ryan Purse
 * @date 08/10/2023
 */


#pragma once

#include "Pch.h"
#include "Drawable.h"
#include "Profiler.h"

namespace engine
{
    /**
     * @author Ryan Purse
     * @date 08/10/2023
     */
    class ProfilerViewer
        : public ui::Drawable
    {
    public:
    protected:
        void onDrawUi() override;
        void drawNode(const debug::ProfileNode &node);
        
        float mUpdateRate { 0.01f };
    };
}

