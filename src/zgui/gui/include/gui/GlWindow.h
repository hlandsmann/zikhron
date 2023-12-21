#pragma once
#include "MainWindow.h"

#include <context/Fonts.h>
#include <context/GlfwImguiContext.h>
#include <context/Theme.h>
#include <imgui.h>
#include <multimedia/MpvWrapper.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <widgets/Box.h>
#include <widgets/Widget.h>
#include <widgets/Window.h>

#include <kocoro/kocoro.hpp>
#include <memory>

class GlWindow
{
public:
    GlWindow(std::shared_ptr<kocoro::SynchronousExecutor>,
             std::shared_ptr<context::GlfwImguiContext> glfwImguiContext,
             MainWindow _mainWindow);
    void run();
    [[nodiscard]] auto shouldClose() const -> bool;

private:
    std::shared_ptr<context::GlfwImguiContext> glfwImguiContext;

    bool close{false};
    constexpr static ImVec4 bgColor = {0.2F, 0.2F, 0.2F, 1.0F};

    MainWindow mainWindow;

    std::shared_ptr<kocoro::SynchronousExecutor> synchronousExecutor;
    std::shared_ptr<MpvWrapper> videoPlayer;
};
