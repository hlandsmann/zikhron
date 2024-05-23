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
        if (logMessages.size() < 1024) {
            logMessages.push_back(std::move(logMessage));
        }
    }

    static void renderLogMessages(bool render)
    {
        static int windowFocusDelay = 0;

        ImGui::Begin("logwindow", nullptr, 0);
        if (!ImGui::IsWindowFocused()) {
            if (windowFocusDelay == 0) {
                windowFocusDelay = 20;
            } else {
                windowFocusDelay--;
            }
            if (windowFocusDelay == 0) {
                ImGui::SetWindowFocus();
            }
        }
        if (render) {
            for (const auto& logMessage : logMessages) {
                ImGui::Text("%s", logMessage.c_str());
            }
        }
        logMessages.clear();
        ImGui::End();
    }

private:
    static std::vector<std::string> logMessages;
};
