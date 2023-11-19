#include <GLFW/glfw3.h>
#include <GlfwImguiContext.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <stdexcept>

namespace {
void glfw_error_callback(int error, const char* description)
{
    spdlog::error("GLFW Error {}: {}\n", error, description);
}
} // namespace

GlfwImguiContext::GlfwImguiContext()
{
    initOpenglContext();
    initImGui();
}

GlfwImguiContext::~GlfwImguiContext()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
}

auto GlfwImguiContext::getGLFWwindow() const -> GLFWwindow*
{
    return glfwWindow;
}

void GlfwImguiContext::initOpenglContext()
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

void GlfwImguiContext::initImGui()
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
