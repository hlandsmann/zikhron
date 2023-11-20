#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include "GlfwImguiContext.h"

#include <GL/gl.h>
#include <Texture.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <map>
#include <memory>
#include <string>

namespace context {
Texture::Texture(std::shared_ptr<GlfwImguiContext> /* glfwImguiContext */)
    : textureMap{loadTextureMap()}
{}

auto Texture::get(Image image) const -> const TextureData&
{
    return textureMap.at(image);
}

auto Texture::loadTextureFromFile(const std::filesystem::path& imageFile) -> TextureData
{
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(imageFile.c_str(), &image_width, &image_height, nullptr, 4);
    if (image_data == nullptr) {
        spdlog::error("Failed to load '{}'", std::string{imageFile});
        return {};
    }

    // Create a OpenGL texture identifier
    GLuint image_texture{};
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    return {.width = image_width,
            .height = image_height,
            .data = image_texture};
}

auto Texture::loadTextureMap() -> std::map<Image, TextureData>
{
    std::map<Image, TextureData> textureMap;
    textureMap[Image::cards] = loadTextureFromFile(cards_tex);
    return textureMap;
}
} // namespace context
