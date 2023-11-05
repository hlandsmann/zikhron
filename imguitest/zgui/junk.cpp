// clang-format off
#include <epoxy/glx.h>
#include <epoxy/egl.h>
#include <epoxy/gl_generated.h>
// clang-format on
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <MainWindow.h>
#include <MediaPlayer.h>
#include <TextDraw.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#include <spdlog/spdlog.h>
auto imain() -> int
{
    // // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    GLFWwindow* window{nullptr};
    std::string glsl_version{"#version 131"};
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version.c_str());
    io.Fonts->AddFontFromFileTTF("/usr/share/fonts/arphicfonts/gkai00mp.ttf", 20, nullptr,
                                 io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());

    // Our state
    bool show_demo_window = false;
    bool show_another_window = false;
    constexpr float bgColorVal = 0.1F;
    ImVec4 clear_color = ImVec4(bgColorVal, bgColorVal, bgColorVal, 1.00F);
    auto mediaPlayer = std::make_unique<MediaPlayer>();
    bool play{false};
    mediaPlayer->initGL();
    mediaPlayer->openFile("/home/harmen/Videos/chinesisch/Cute Programmer E01 1080p WEB-DL AAC H.264-Luvmichelle.mkv");
    // mediaPlayer->play();
    while (glfwWindowShouldClose(window) == 0) {
        glfwPollEvents();
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0F;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.
            ImGui::SetCursorPos({100, 100});
            ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
            // ImGui::SetCursorPos({50,50});
            ImGui::Checkbox("Another Window", &show_another_window);
            ImGui::Checkbox("play", &play);

            ImGui::SliderFloat("float", &f, 0.0F, 1.0F); // Edit 1 float using a slider from 0.0f to 1.0f
            // ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) { // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            }
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window) {
            // Pass a pointer to our bool variable (the window will have a closing button that will
            // clear the bool when clicked)
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me")) {
                show_another_window = false;
            }
            ImGui::End();
        }
        ImGui::SetNextWindowPos({10, 10});
        ImGui::GetItemRectMin();
        ImGui::Begin("Window Name", nullptr,
                     ImGuiWindowFlags_NoTitleBar
                             | ImGuiWindowFlags_NoBackground
                             | ImGuiWindowFlags_NoMove
                             | ImGuiWindowFlags_NoResize);
        ImGui::TextWithHoverColor({1.0F, 0.0F, 0.0F, 1.0F}, "之前");
        ImGui::Button("##Button", ImVec2(20, 20));
        ImGui::End();
        // Rendering
        ImGui::Render();
        int display_w{};
        int display_h{};
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        if (play) {
            mediaPlayer->play();
            mediaPlayer->render(display_w - 40, display_h);
        } else {
            mediaPlayer->pause();
        }
        // glBlitFramebuffer(
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
