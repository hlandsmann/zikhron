#pragma once
#include <context/GlfwImguiContext.h>
#include "MainWindow.h"

#include <context/Theme.h>
#include <folly/executors/ManualExecutor.h>
#include <imgui.h>
#include <multimedia/MpvWrapper.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <widgets/Box.h>
#include <context/Fonts.h>
#include <widgets/Widget.h>
#include <widgets/Window.h>

#include <memory>
#include <string>

class GlWindow
{
public:
    GlWindow(std::shared_ptr<folly::ManualExecutor> synchronousExecutor,
             std::shared_ptr<GlfwImguiContext> glfwImguiContext,
             MainWindow _mainWindow);
    void run();
    [[nodiscard]] auto shouldClose() const -> bool;

private:
    std::shared_ptr<GlfwImguiContext> glfwImguiContext;

    bool close{false};
    constexpr static ImVec4 bgColor = {0.2F, 0.2F, 0.2F, 1.0F};

    MainWindow mainWindow;

    std::shared_ptr<folly::ManualExecutor> synchronousExecutor;
    std::shared_ptr<MpvWrapper> videoPlayer;
};
