
// folly headers must be included before epoxy/glx.h
// #include <folly/Executor.h>
// epoxy/glx must be included before opengl headers can be included
// #include <epoxy/glx.h>
// #include <epoxy/egl.h>
// #include <epoxy/gl_generated.h>
// clang-format on
#include <Fonts.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <MainWindow.h>
#include <MediaPlayer.h>
#include <SideBar.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ManualExecutor.h>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <widget/Layout.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
namespace {
void glfw_error_callback(int error, const char* description)
{
    spdlog::error("GLFW Error {}: {}\n", error, description);
}
} // namespace

MainWindow::MainWindow(std::shared_ptr<folly::ManualExecutor> _executor)
    : executor{std::move(_executor)}
    , threadPoolExecutor{std::make_shared<folly::CPUThreadPoolExecutor>(
              0, std::thread::hardware_concurrency())}
    , videoPlayer{std::make_shared<MediaPlayer>()}
{
    initOpenglContext();
    initImGui();
    fonts = std::make_unique<Fonts>();
    videoPlayer->initGL();
    // videoPlayer->openFile("/home/harmen/Videos/chinesisch/Cute Programmer E01 1080p WEB-DL AAC H.264-Luvmichelle.mkv");
    // videoPlayer->play();
}

MainWindow::~MainWindow()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void MainWindow::initOpenglContext()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (glfwInit() == 0) {
        throw std::runtime_error("glflInit failed!");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    window = glfwCreateWindow(1980, 1200, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("Failed to create Window");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
}

void MainWindow::initImGui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version.c_str());
    io.Fonts->AddFontFromFileTTF("/usr/share/fonts/arphicfonts/gkai00mp.ttf", 20, nullptr,
                                 io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
}

void MainWindow::doImGui(const widget::layout::rect& rect)
{
    bool show_demo_window = true;
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }
    sideBar.doChoice({.x = rect.width - 80, .y = 0, .width = 80, .height = rect.height});

    ImGui::PushFont(fonts->ChineseBig());
    ImGui::Text("位置");
    ImGui::PopFont();

    ImGui::PushFont(fonts->ChineseSmall());
    ImGui::Text("位置");
    ImGui::PopFont();

    ImGui::PushFont(fonts->Gui());
    ImGui::Text("Hello WOrld");
    ImGui::PopFont();

    using Align = widget::layout::Align;
    window1 = layout.add<widget::Window>(Align::start,0,0,"win1"); 
}

void MainWindow::run()
{
    int display_w{};
    int display_h{};
    glfwGetFramebufferSize(window, &display_w, &display_h);

    widget::layout::rect rect{0, 0, static_cast<float>(display_w), static_cast<float>(display_h)};

    if (glfwWindowShouldClose(window) != 0) {
        close = true;
        return;
    }
    glfwPollEvents();
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    doImGui(rect);
    ImGui::Render();

    glViewport(0, 0, display_w, display_h);
    glClearColor(bgColor.x * bgColor.w,
                 bgColor.y * bgColor.w,
                 bgColor.z * bgColor.w,
                 bgColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    videoPlayer->render(display_w - 80, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

auto MainWindow::shouldClose() const -> bool
{
    return close;
}
