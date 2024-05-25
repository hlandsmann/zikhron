// clang-format on
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <GlWindow.h>
#include <MainWindow.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <context/Fonts.h>
#include <context/GlfwImguiContext.h>
#include <context/Theme.h>
#include <context/imglog.h>
#include <imgui.h>
#include <multimedia/MpvWrapper.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spdlog/spdlog.h>
#include <widgets/Box.h>
#include <widgets/Button.h>
#include <widgets/detail/Widget.h>

#include <kocoro/kocoro.hpp>
#include <memory>
#include <utility>

namespace gui {

GlWindow::GlWindow(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
                   std::shared_ptr<context::GlfwImguiContext> _glfwImguiContext,
                   std::unique_ptr<MainWindow> _mainWindow)
    : glfwImguiContext{std::move(_glfwImguiContext)}
    , mainWindow{std::move(_mainWindow)}
    , executor{std::move(_synchronousExecutor)}
{
    mainWindow->setup();
}

void GlWindow::run()
{
    while (true) {
        const auto& [width, height] = startImGuiFrame();
        if (shouldClose()) {
            break;
        }

        executor->run();

        widget::layout::Rect rect{.x = 0, .y = 0, .width = static_cast<float>(width), .height = static_cast<float>(height)};

        {
            // ImGuiStyle& style = ImGui::GetStyle();
            // style.FrameBorderSize = 1.F;
            setStyleVars();

            // auto styleVarsDrop = context::Theme::dropImGuiStyleVars();
            mainWindow->arrange(rect);
            mainWindow->doImGui();
        }
        // imglog::renderLogMessages(true);

        finishFrame();

        render();
    }
}

auto GlWindow::shouldClose() const -> bool
{
    return close;
}

auto GlWindow::startImGuiFrame() -> std::pair<int, int>
{
    glfwGetFramebufferSize(glfwImguiContext->getGLFWwindow(), &displayWidth, &displayHeight);

    if (glfwWindowShouldClose(glfwImguiContext->getGLFWwindow()) != 0) {
        close = true;
        return {};
    }
    glfwPollEvents();
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    return {displayWidth, displayHeight};
}

void GlWindow::finishFrame() const
{
    ImGui::Render();

    glViewport(0, 0, displayWidth, displayHeight);
    glClearColor(bgColor.x * bgColor.w,
                 bgColor.y * bgColor.w,
                 bgColor.z * bgColor.w,
                 bgColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GlWindow::render()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(glfwImguiContext->getGLFWwindow());
}

void GlWindow::setStyleVars()
{
    ImVec4* colors = ImGui::GetStyle().Colors;
    // colors[ImGuiCol_Text] = ImVec4(1.00F, 1.00F, 1.00F, 1.00F);
    // colors[ImGuiCol_TextDisabled] = ImVec4(0.50F, 0.50F, 0.50F, 1.00F);
    // colors[ImGuiCol_WindowBg] = ImVec4(0.10F, 0.10F, 0.10F, 1.00F);
    // colors[ImGuiCol_ChildBg] = ImVec4(0.00F, 0.00F, 0.00F, 0.00F);
    // colors[ImGuiCol_PopupBg] = ImVec4(0.19F, 0.19F, 0.19F, 0.92F);
    // colors[ImGuiCol_Border] = ImVec4(0.19F, 0.19F, 0.19F, 0.29F);
    // colors[ImGuiCol_BorderShadow] = ImVec4(0.00F, 0.00F, 0.00F, 0.24F);
    colors[ImGuiCol_FrameBg] = ImVec4(0.08F, 0.08F, 0.08F, 1.F);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.11F, 0.11F, 0.11F, 1.F);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.13F, 0.13F, 0.13F, 1.00F);
    // colors[ImGuiCol_TitleBg] = ImVec4(0.00F, 0.00F, 0.00F, 1.00F);
    // colors[ImGuiCol_TitleBgActive] = ImVec4(0.06F, 0.06F, 0.06F, 1.00F);
    // colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00F, 0.00F, 0.00F, 1.00F);
    // colors[ImGuiCol_MenuBarBg] = ImVec4(0.14F, 0.14F, 0.14F, 1.00F);
    // colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05F, 0.05F, 0.05F, 0.54F);
    // colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34F, 0.34F, 0.34F, 0.54F);
    // colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40F, 0.40F, 0.40F, 0.54F);
    // colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56F, 0.56F, 0.56F, 0.54F);
    // colors[ImGuiCol_CheckMark] = ImVec4(0.33F, 0.67F, 0.86F, 1.00F);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.34F, 0.34F, 0.34F, 1.F);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.46F, 0.46F, 0.45F, 1.F);
    // colors[ImGuiCol_Button] = ImVec4(0.05F, 0.05F, 0.05F, 0.54F);
    // colors[ImGuiCol_ButtonHovered] = ImVec4(0.19F, 0.19F, 0.19F, 0.54F);
    // colors[ImGuiCol_ButtonActive] = ImVec4(0.20F, 0.22F, 0.23F, 1.00F);
    // colors[ImGuiCol_Header] = ImVec4(0.00F, 0.00F, 0.00F, 0.52F);
    // colors[ImGuiCol_HeaderHovered] = ImVec4(0.00F, 0.00F, 0.00F, 0.36F);
    // colors[ImGuiCol_HeaderActive] = ImVec4(0.20F, 0.22F, 0.23F, 0.33F);
    // colors[ImGuiCol_Separator] = ImVec4(0.28F, 0.28F, 0.28F, 0.29F);
    // colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44F, 0.44F, 0.44F, 0.29F);
    // colors[ImGuiCol_SeparatorActive] = ImVec4(0.40F, 0.44F, 0.47F, 1.00F);
    // colors[ImGuiCol_ResizeGrip] = ImVec4(0.28F, 0.28F, 0.28F, 0.29F);
    // colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44F, 0.44F, 0.44F, 0.29F);
    // colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40F, 0.44F, 0.47F, 1.00F);
    // colors[ImGuiCol_Tab] = ImVec4(0.00F, 0.00F, 0.00F, 0.52F);
    // colors[ImGuiCol_TabHovered] = ImVec4(0.14F, 0.14F, 0.14F, 1.00F);
    // colors[ImGuiCol_TabActive] = ImVec4(0.20F, 0.20F, 0.20F, 0.36F);
    // colors[ImGuiCol_TabUnfocused] = ImVec4(0.00F, 0.00F, 0.00F, 0.52F);
    // colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14F, 0.14F, 0.14F, 1.00F);
    // // colors[ImGuiCol_DockingPreview]         = ImVec4(0.33F, 0.67F, 0.86F, 1.00F);
    // // colors[ImGuiCol_DockingEmptyBg]         = ImVec4(1.00F, 0.00F, 0.00F, 1.00F);
    // colors[ImGuiCol_PlotLines] = ImVec4(1.00F, 0.00F, 0.00F, 1.00F);
    // colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00F, 0.00F, 0.00F, 1.00F);
    // colors[ImGuiCol_PlotHistogram] = ImVec4(1.00F, 0.00F, 0.00F, 1.00F);
    // colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00F, 0.00F, 0.00F, 1.00F);
    // colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00F, 0.00F, 0.00F, 0.52F);
    // colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00F, 0.00F, 0.00F, 0.52F);
    // colors[ImGuiCol_TableBorderLight] = ImVec4(0.28F, 0.28F, 0.28F, 0.29F);
    // colors[ImGuiCol_TableRowBg] = ImVec4(0.00F, 0.00F, 0.00F, 0.00F);
    // colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00F, 1.00F, 1.00F, 0.06F);
    // colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20F, 0.22F, 0.23F, 1.00F);
    // colors[ImGuiCol_DragDropTarget] = ImVec4(0.33F, 0.67F, 0.86F, 1.00F);
    // colors[ImGuiCol_NavHighlight] = ImVec4(1.00F, 0.00F, 0.00F, 1.00F);
    // colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00F, 0.00F, 0.00F, 0.70F);
    // colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00F, 0.00F, 0.00F, 0.20F);
    // colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00F, 0.00F, 0.00F, 0.35F);

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(0.00F, 0.00F);
    style.FramePadding = ImVec2(0.00F, 0.00F);
    // style.CellPadding = ImVec2(6.00F, 6.00F);
    // style.ItemSpacing = ImVec2(6.00F, 6.00F);
    // style.ItemInnerSpacing = ImVec2(6.00F, 6.00F);
    // style.TouchExtraPadding = ImVec2(0.00F, 0.00F);
    // style.IndentSpacing = 25;
    // style.ScrollbarSize = 15;
    // style.GrabMinSize = 10;
    style.WindowBorderSize = 0;
    // style.ChildBorderSize = 1;
    // style.PopupBorderSize = 1;
    // style.FrameBorderSize = 1;
    // style.TabBorderSize = 1;
    // style.WindowRounding = 7;
    // style.ChildRounding = 4;
    // style.FrameRounding = 3;
    // style.PopupRounding = 4;
    // style.ScrollbarRounding = 9;
    // style.GrabRounding = 3;
    // style.LogSliderDeadzone = 4;
    // style.TabRounding = 4;
}

} // namespace gui
