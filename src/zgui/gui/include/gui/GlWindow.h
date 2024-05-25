#pragma once
#include "MainWindow.h"

#include <context/Drop.h>
#include <context/Fonts.h>
#include <context/GlfwImguiContext.h>
#include <context/Theme.h>
#include <imgui.h>
#include <multimedia/MpvWrapper.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <utils/spdlog.h>
#include <widgets/Box.h>
#include <widgets/Window.h>
#include <widgets/detail/Widget.h>

#include <kocoro/kocoro.hpp>
#include <memory>
#include <utility>

namespace gui {

class GlWindow
{
public:
    GlWindow(std::shared_ptr<kocoro::SynchronousExecutor>,
             std::shared_ptr<context::GlfwImguiContext> glfwImguiContext,
             std::unique_ptr<MainWindow> mainWindow);

    GlWindow(const GlWindow&) = delete;
    GlWindow(GlWindow&&) = delete;
    auto operator=(const GlWindow&) -> GlWindow& = delete;
    auto operator=(GlWindow&&) -> GlWindow& = delete;
    virtual ~GlWindow() = default;
    void run();
    [[nodiscard]] auto shouldClose() const -> bool;

private:
    auto startImGuiFrame() -> std::pair<int, int>;
    void finishFrame() const;
    void render();
    void setStyleVars();
    std::shared_ptr<context::GlfwImguiContext> glfwImguiContext;

    bool close{false};
    constexpr static ImVec4 bgColor = {0.1F, 0.1F, 0.1F, 1.0F};

    std::unique_ptr<MainWindow> mainWindow;

    std::shared_ptr<kocoro::SynchronousExecutor> executor;
    int displayWidth{};
    int displayHeight{};
};

} // namespace gui
