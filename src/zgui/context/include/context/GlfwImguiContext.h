#pragma once
#include <GLFW/glfw3.h>

#include <string>

class GlfwImguiContext
{
public:
    GlfwImguiContext();
    ~GlfwImguiContext();
    GlfwImguiContext(const GlfwImguiContext&) = default;
    GlfwImguiContext(GlfwImguiContext&&) = default;
    auto operator=(const GlfwImguiContext&) -> GlfwImguiContext& = default;
    auto operator=(GlfwImguiContext&&) -> GlfwImguiContext& = default;
    [[nodiscard]] auto getGLFWwindow() const -> GLFWwindow*;

private:
    void initOpenglContext();
    void initImGui();
    GLFWwindow* glfwWindow{nullptr};
    std::string glsl_version{"#version 130"};
};
