#pragma once
#include <imgui.h>

#include <format>
#include <string>
#include <utility>
#include <vector>

class imglog
{
public:
    template<class... Args>
    static void log(std::format_string<Args...> fmt, Args&&... args)
    {
        std::string logMessage = std::format(fmt, std::forward<Args>(args)...);
        logMessages.push_back(std::move(logMessage));
    }

    static void renderLogMessages()
    {
        ImGui::Begin("logwindow", nullptr,
                     0);
        for (const auto& logMessage : logMessages) {
            ImGui::Text("%s", logMessage.c_str());
        }
        logMessages.clear();
        ImGui::End();
    }

private:
    static std::vector<std::string> logMessages;
};
