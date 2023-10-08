/**
 * @file LogWindow.h
 * @author Ryan Purse
 * @date 20/08/2023
 */


#pragma once

#include "Pch.h"
#include "Drawable.h"

namespace engine
{
/**
 * @author Ryan Purse
 * @date 20/08/2023
 */
    class LogWindow
        : public ui::Drawable
    {
    protected:
        void onDrawUi() override;
        void drawMessageUi(const debug::Message &message);
        const ImVec4 &getSeverityColour(debug::Severity severity);
        
    protected:
        bool mWrapText          { true };
        bool mShowNotifications { true };
        bool mShowWarnings      { true };
        bool mShowErrors        { true };
        bool mCollapse          { true };
        
        ImVec4 mNotificationColour  { 1.f, 1.f, 1.f, 1.f };
        ImVec4 mWarningColour       { 1.f, 1.f, 0.f, 1.f };
        ImVec4 mErrorColour         { 1.f, 0.f, 0.f, 1.f };
        
        std::set<std::string> mUniqueMessages;
    };
    
} // engine
