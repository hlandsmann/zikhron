#pragma once
#include "CardDisplay.h"
#include "SideBar.h"

#include <GLFW/glfw3.h>
#include <folly/executors/ManualExecutor.h>
#include <imgui.h>
#include <multimedia/MpvWrapper.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <widgets/Box.h>
#include <widgets/Fonts.h>
#include <widgets/Widget.h>
#include <widgets/Window.h>

#include <memory>
#include <string>

class MainWindow
{
public:
    MainWindow(std::shared_ptr<folly::ManualExecutor> executor,
               std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker);
    ~MainWindow();
    MainWindow(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    auto operator=(const MainWindow&) -> MainWindow& = delete;
    auto operator=(MainWindow&&) -> MainWindow& = delete;
    void run();
    [[nodiscard]] auto shouldClose() const -> bool;

private:
    void initOpenglContext();
    void initImGui();
    void doImGui(const widget::layout::Rect& rect);
    void arrangeLayout();
    std::string glsl_version{"#version 130"};
    bool close{false};
    GLFWwindow* glfwWindow{nullptr};
    constexpr static ImVec4 bgColor = {0.2F, 0.2F, 0.2F, 1.0F};

    // ImGui widgets
    SideBar sideBar{};
    std::unique_ptr<Fonts> fonts;
    widget::Box layout{widget::layout::Orientation::horizontal};

    // async
    std::shared_ptr<folly::ManualExecutor> synchronousExecutor;
    std::shared_ptr<sr::AsyncTreeWalker> asyncTreeWalker;
    std::shared_ptr<MpvWrapper> videoPlayer;

    CardDisplay cardDisplay;
};
