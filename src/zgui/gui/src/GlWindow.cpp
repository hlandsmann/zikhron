// clang-format on
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <GlWindow.h>
#include <MainWindow.h>
#include <SideBar.h>
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

GlWindow::GlWindow(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
                   std::shared_ptr<context::GlfwImguiContext> _glfwImguiContext,
                   MainWindow _mainWindow)
    : glfwImguiContext{std::move(_glfwImguiContext)}
    , mainWindow{std::move(_mainWindow)}
    , executor{std::move(_synchronousExecutor)}
    , videoPlayer{std::make_shared<MpvWrapper>()}
{
    videoPlayer->initGL();
    mainWindow.setup();
    // videoPlayer->openFile("/home/harmen/Videos/chinesisch/Cute Programmer E01 1080p WEB-DL AAC H.264-Luvmichelle.mkv");
    // videoPlayer->play();
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
            auto styleVarsDrop = context::Theme::dropImGuiStyleVars();
            mainWindow.arrange(rect);
            mainWindow.doImGui();
        }
        imglog::renderLogMessages(true);

        finishFrame();

        videoPlayer->render(width, height);
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
