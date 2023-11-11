#pragma once
#include <widget/Widget.h>
#include <Fonts.h>
#include <GLFW/glfw3.h>
#include <MediaPlayer.h>
#include <SideBar.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ManualExecutor.h>
#include <imgui.h>
#include <widget/Layout.h>
#include <widget/Window.h>

#include <memory>
#include <string>

class MainWindow
{
public:
    MainWindow(std::shared_ptr<folly::ManualExecutor> executor);
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
    GLFWwindow* window{nullptr};
    constexpr static ImVec4 bgColor = {0.1F, 0.1F, 0.1F, 1.0F};

    // ImGui widgets
    SideBar sideBar{};
    std::unique_ptr<Fonts> fonts;
    widget::Layout layout{widget::layout::Orientation::horizontal};

    // async
    std::shared_ptr<folly::ManualExecutor> executor;
    std::shared_ptr<folly::CPUThreadPoolExecutor> threadPoolExecutor;
    std::shared_ptr<MediaPlayer> videoPlayer;
};
