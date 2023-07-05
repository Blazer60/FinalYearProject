/**
 * @file PropertyDrawer.h
 * @author Ryan Purse
 * @date 15/07/2023
 */


#pragma once

#include "Pch.h"
#include "imgui.h"

namespace ui
{
    /**
     * An interface to tell the ui that this item can be drawn via imgui and expose properties that a user can change.
     * Use ui::draw(property) to safely draw an element that implements Drawable.
     * @author Ryan Purse
     * @date 15/07/2023
     */
    class Drawable
    {
    public:
        virtual ~Drawable() = default;
        
        void drawUi();
    protected:
        virtual void onDrawUi() = 0;
    };
    
    template<typename T>
    void draw(T &property)
    {
        try
        {
            auto* drawable = dynamic_cast<Drawable*>(&property);
            if (drawable)
                drawable->drawUi();
        }
        catch (const std::bad_cast &badCast)
        {
            debug::log(badCast.what(), debug::severity::Minor);
        }
    }
    
    template<typename T>
    void draw(T *property)
    {
        draw(*property);
    }
}
