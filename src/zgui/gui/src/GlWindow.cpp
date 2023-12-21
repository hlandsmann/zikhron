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
#include <context/imglog.h>
#include <imgui.h>
#include <multimedia/MpvWrapper.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spdlog/spdlog.h>
#include <widgets/Box.h>
#include <widgets/Button.h>
#include <widgets/Widget.h>

#include <kocoro/kocoro.hpp>
#include <memory>
#include <utility>

GlWindow::GlWindow(std::shared_ptr<kocoro::SynchronousExecutor> _synchronousExecutor,
                   std::shared_ptr<context::GlfwImguiContext> _glfwImguiContext,
                   MainWindow _mainWindow)
    : glfwImguiContext{std::move(_glfwImguiContext)}
    , mainWindow{std::move(_mainWindow)}
    , synchronousExecutor{std::move(_synchronousExecutor)}
    , videoPlayer{std::make_shared<MpvWrapper>()}
{
    videoPlayer->initGL();
    mainWindow.arrangeLayout();
    // videoPlayer->openFile("/home/harmen/Videos/chinesisch/Cute Programmer E01 1080p WEB-DL AAC H.264-Luvmichelle.mkv");
    // videoPlayer->play();
}

void GlWindow::run()
{
    int display_w{};
    int display_h{};
    glfwGetFramebufferSize(glfwImguiContext->getGLFWwindow(), &display_w, &display_h);

    widget::layout::Rect rect{0, 0, static_cast<float>(display_w), static_cast<float>(display_h)};

    if (glfwWindowShouldClose(glfwImguiContext->getGLFWwindow()) != 0) {
        close = true;
        return;
    }
    glfwPollEvents();
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    mainWindow.doImGui(rect);
    imglog::renderLogMessages();
    ImGui::Render();

    glViewport(0, 0, display_w, display_h);
    glClearColor(bgColor.x * bgColor.w,
                 bgColor.y * bgColor.w,
                 bgColor.z * bgColor.w,
                 bgColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    videoPlayer->render(display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(glfwImguiContext->getGLFWwindow());
}

auto GlWindow::shouldClose() const -> bool
{
    return close;
}
