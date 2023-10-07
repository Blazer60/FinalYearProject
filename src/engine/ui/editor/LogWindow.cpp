/**
 * @file LogWindow.cpp
 * @author Ryan Purse
 * @date 20/08/2023
 */


#include "LogWindow.h"
#include "Ui.h"
#include "ProfileTimer.h"

namespace engine
{
    void LogWindow::onDrawUi()
    {
        PROFILE_FUNC();
        ImGui::PushID("Logging");
        ImGui::Begin("Log Window");
        
        if (ImGui::Button("Clear"))
            engine::logger->clearQueue();
        ui::drawToolTip("Clears the log queue but does not clear the IO log.");
        ImGui::SameLine();
        ImGui::Checkbox("Wrap Text", &mWrapText);
        ImGui::SameLine();
        ImGui::Checkbox("Show Notifications", &mShowNotifications);
        ImGui::SameLine();
        ImGui::Checkbox("Show Warnings", &mShowWarnings);
        ImGui::SameLine();
        ImGui::Checkbox("Show Errors", &mShowErrors);
        ImGui::SameLine();
        ImGui::Checkbox("Collapse", &mCollapse);
        ui::drawToolTip("Messages from the same source are collapsed into one message. If ticked, only the most recent message is shown.");
        
        mUniqueMessages.clear();
        
        if (ImGui::BeginTable("Log Table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn("Severity");
            ImGui::TableSetupColumn("Details");
            ImGui::TableHeadersRow();
            ImGui::PushTextWrapPos(mWrapText ? 0.f : -1.f);
            
            const std::vector<Message> &logs = engine::logger->getLogs();
            for (auto it = logs.rbegin(); it != logs.rend(); ++it)
                drawMessageUi(*it);
            
            ImGui::PopTextWrapPos();
            ImGui::EndTable();
        }
        
        ImGui::End();
        ImGui::PopID();
    }
    
    void LogWindow::drawMessageUi(const Message &message)
    {
        PROFILE_FUNC();
        const std::string id = std::to_string(message.line) + message.file;
        if (mCollapse)
        {
            if (mUniqueMessages.count(id) > 0)
                return;
        }
        
        if (message.severity == Severity_Notification && mShowNotifications ||
            message.severity == Severity_Warning && mShowWarnings ||
            message.severity == Severity_Major && mShowErrors)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Text, getSeverityColour(message.severity));
            ImGui::Text("%s", engine::logger->toStringView(message.severity).data());
            ImGui::TableNextColumn();
            ImGui::Text("%s", message.message.c_str());
            ImGui::PopStyleColor();
            
            if (mCollapse)
                mUniqueMessages.emplace(id);
        }
    }
    
    const ImVec4 &LogWindow::getSeverityColour(const Severity severity)
    {
        switch (severity)
        {
            default:
            case Severity_Notification:
                return mNotificationColour;
            case Severity_Warning:
                return mWarningColour;
            case Severity_Major:
                return mErrorColour;
        }
    }
}
