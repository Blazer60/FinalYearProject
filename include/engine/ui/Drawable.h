/**
 * @file Drawable.h
 * @author Ryan Purse
 * @date 15/07/2023
 */


#pragma once

#include "Pch.h"
#include "imgui.h"
#include <Statistics.h>

namespace ui
{
    /**
     * An interface to tell the ui that this item can be drawn via drawUi and expose properties that a user can change.
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
            if (auto* drawable = dynamic_cast<Drawable*>(&property))
                drawable->drawUi();
        }
        catch (const std::bad_cast &badCast)
        {
            LOG_MINOR(badCast.what());
        }
    }
    
    template<typename T>
    void draw(T *property)
    {
        draw(*property);
    }
    
    template<typename T>
    void draw(std::unique_ptr<T> &property)
    {
        draw(property.get());
    }

    template<typename T>
    void draw(std::shared_ptr<T> &property)
    {
        draw(property.get());
    }
}
