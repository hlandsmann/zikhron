
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
#include <widget/Button.h>
#include <widget/Box.h>
#include <widget/Widget.h>
#include <widget/imglog.h>

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
    arrangeLayout();
    // videoPlayer->openFile("/home/harmen/Videos/chinesisch/Cute Programmer E01 1080p WEB-DL AAC H.264-Luvmichelle.mkv");
    // videoPlayer->play();
}

MainWindow::~MainWindow()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(glfwWindow);
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
    glfwWindow = glfwCreateWindow(1980, 1200, "zikhron", nullptr, nullptr);
    if (glfwWindow == nullptr) {
        throw std::runtime_error("Failed to create Window");
    }
    glfwMakeContextCurrent(glfwWindow);
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
    ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
    ImGui_ImplOpenGL3_Init(glsl_version.c_str());
    // io.Fonts->AddFontFromFileTTF("/usr/share/fonts/arphicfonts/gkai00mp.ttf", 20, nullptr,
    //                              io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
}

void MainWindow::doImGui(const widget::layout::Rect& rect)
{
    // sideBar.doChoice({.x = rect.width - 80, .y = 0, .width = 80, .height = rect.height});
    // ImGui::PushFont(fonts->Gui());
    layout.arrange(rect);
    {
        auto droppedWindow = layout.next<widget::Window>().dropWindow();
        ImGui::PushFont(fonts->ChineseBig());
        ImGui::Text("位置");
        ImGui::Text("1");
        ImGui::PopFont();
        // }
        // {
        //     auto droppedWindow = layout.next<widget::Window>().dropWindow();
        ImGui::PushFont(fonts->ChineseSmall());
        ImGui::Text("位置");
        ImGui::Text("2");
        ImGui::PopFont();
        // }
        // {
        //     auto droppedWindow = layout.next<widget::Window>().dropWindow();
        ImGui::PushFont(fonts->Gui());
        ImGui::Text("Hello World");
        ImGui::Text("3");
        ImGui::PopFont();
    }
    {
        auto& window = layout.next<widget::Window>();
        auto droppedWindow = window.dropWindow();
        window.getLayout().next<widget::Button>().clicked();
        window.getLayout().next<widget::Button>().clicked();
        window.getLayout().next<widget::Button>().clicked();

        // ImGui::PushFont(fonts->Gui());
        // ImGui::Text("Hello WOrld");
        // ImGui::Text("4");
        // ImGui::PopFont();
    }
    // ImGui::PopFont();
    bool show_demo_window = true;
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }
}

void MainWindow::arrangeLayout()
{
    using Align = widget::layout::Align;
    using namespace widget::layout;

    layout.add<widget::Window>(Align::start, width_expand, height_expand, "win1");
    // layout.add<widget::Window>(Align::center, 80, 0, width_fixed, height_expand, "win2");
    // layout.add<widget::Window>(Align::center, 80, 0, width_expand, height_expand, "win3");
    auto& window = *layout.add<widget::Window>(Align::end, width_fixed, height_expand, "win4");
    window.getLayout().add<widget::Button>(Align::start, "Cards");
    window.getLayout().add<widget::Button>(Align::start, "Video");
    window.getLayout().add<widget::Button>(Align::start, "Audio Group");
}

void MainWindow::run()
{
    int display_w{};
    int display_h{};
    glfwGetFramebufferSize(glfwWindow, &display_w, &display_h);

    widget::layout::Rect rect{0, 0, static_cast<float>(display_w), static_cast<float>(display_h)};

    if (glfwWindowShouldClose(glfwWindow) != 0) {
        close = true;
        return;
    }
    glfwPollEvents();
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    doImGui(rect);
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

    glfwSwapBuffers(glfwWindow);
}

auto MainWindow::shouldClose() const -> bool
{
    return close;
}
