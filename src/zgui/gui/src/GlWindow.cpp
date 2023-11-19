
// folly headers must be included before epoxy/glx.h
// #include <folly/Executor.h>
// epoxy/glx must be included before opengl headers can be included
// #include <epoxy/glx.h>
// #include <epoxy/egl.h>
// #include <epoxy/gl_generated.h>
// clang-format on
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <GlWindow.h>
#include <MainWindow.h>
#include <SideBar.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ManualExecutor.h>
#include <imgui.h>
#include <multimedia/MpvWrapper.h>
#include <spaced_repetition/AsyncTreeWalker.h>
#include <spdlog/spdlog.h>
#include <widgets/Box.h>
#include <widgets/Button.h>
#include <context/Fonts.h>
#include <widgets/Widget.h>
#include <context/imglog.h>
#include <context/GlfwImguiContext.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

GlWindow::GlWindow(std::shared_ptr<folly::ManualExecutor> _synchronousExecutor,
                   std::shared_ptr<GlfwImguiContext> _glfwImguiContext,
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
