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
        if (!isShowing)
            return;

        ImGui::PushID("Logging");

        if (ImGui::Begin("Log Window", &isShowing))
        {
            if (ImGui::Button("Clear"))
                debug::logger->clearQueue();
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
            ImGui::Checkbox("Show Verbose", &mShowVerbose);
            ImGui::SameLine();
            ImGui::Checkbox("Collapse", &mCollapse);
            ui::drawToolTip("Messages from the same source are collapsed into one message. If ticked, only the most recent message is shown.");

            mUniqueMessages.clear();

            if (ImGui::BeginTable("Log Table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
            {
                ImGui::TableSetupColumn("Line");
                ImGui::TableSetupColumn("File");
                ImGui::TableSetupColumn("Severity");
                ImGui::TableSetupColumn("Details");
                ImGui::TableHeadersRow();
                ImGui::PushTextWrapPos(mWrapText ? 0.f : -1.f);

                const std::vector<debug::Message> &logs = debug::logger->getLogs();
                for (auto it = logs.rbegin(); it != logs.rend(); ++it)
                    drawMessageUi(*it);

                ImGui::PopTextWrapPos();
                ImGui::EndTable();
            }
        }

        ImGui::End();
        ImGui::PopID();
    }
    
    void LogWindow::drawMessageUi(const debug::Message &message)
    {
        const std::string id = std::to_string(message.line) + message.file.string();
        if (mCollapse)
        {
            if (mUniqueMessages.count(id) > 0)
                return;
        }
        
        if (message.severity == debug::Severity_Notification && mShowNotifications ||
            message.severity == debug::Severity_Warning && mShowWarnings ||
            ((message.severity & debug::Severity_Error) > 0 && mShowErrors) ||
            (message.severity & debug::Severity_Verbose) > 0 && mShowVerbose)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Text, getSeverityColour(message.severity));
            ImGui::Text("%i", message.line);
            ImGui::TableNextColumn();
            const std::string fileName = message.file.filename().string();
            ImGui::Text("%s", fileName.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", debug::logger->toStringView(message.severity).data());
            ImGui::TableNextColumn();
            ImGui::Text("%s", message.message.c_str());
            ImGui::PopStyleColor();
            
            if (mCollapse)
                mUniqueMessages.emplace(id);
        }
    }
    
    const ImVec4 &LogWindow::getSeverityColour(const debug::Severity severity)
    {
        switch (severity)
        {
            default:
            case debug::Severity_Notification:
                return mNotificationColour;
            case debug::Severity_Warning:
                return mWarningColour;
            case debug::Severity_Minor:
            case debug::Severity_Major:
                return mErrorColour;
            case debug::Severity_Verbose:
                return mVerboseColour;
        }
    }
}
