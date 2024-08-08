#include "GlfwImguiContext.h"

#include <GL/gl.h>
#include <Texture.h>
#include <spdlog/spdlog.h>
#include <stb/stb_image.h>

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

    return {.width = static_cast<float>(image_width),
            .height = static_cast<float>(image_height),
            .data = image_texture};
}

auto Texture::loadTextureMap() -> std::map<Image, TextureData>
{
    std::map<Image, TextureData> textureMap;
    textureMap[Image::audio] = loadTextureFromFile(audio_tex);
    textureMap[Image::cards] = loadTextureFromFile(cards_tex);
    textureMap[Image::configure_app] = loadTextureFromFile(configure_app_tex);
    textureMap[Image::configure_mid] = loadTextureFromFile(configure_mid_tex);
    textureMap[Image::configure] = loadTextureFromFile(configure_tex);
    textureMap[Image::video] = loadTextureFromFile(video_tex);
    textureMap[Image::document_open] = loadTextureFromFile(document_open_tex);
    textureMap[Image::document_save] = loadTextureFromFile(document_save_tex);
    textureMap[Image::media_playback_pause] = loadTextureFromFile(media_playback_pause_tex);
    textureMap[Image::media_playback_start] = loadTextureFromFile(media_playback_start_tex);
    textureMap[Image::media_playback_stop] = loadTextureFromFile(media_playback_stop_tex);
    textureMap[Image::media_seek_backward] = loadTextureFromFile(media_seek_backward_tex);
    textureMap[Image::media_seek_forward] = loadTextureFromFile(media_seek_forward_tex);
    textureMap[Image::media_skip_backward] = loadTextureFromFile(media_skip_backward_tex);
    textureMap[Image::media_skip_forward] = loadTextureFromFile(media_skip_forward_tex);
    textureMap[Image::new_audio_alarm] = loadTextureFromFile(new_audio_alarm_tex);
    textureMap[Image::arrow_up] = loadTextureFromFile(arrow_up_tex);
    textureMap[Image::arrow_down] = loadTextureFromFile(arrow_down_tex);
    textureMap[Image::arrow_left] = loadTextureFromFile(arrow_left_tex);
    textureMap[Image::arrow_right] = loadTextureFromFile(arrow_right_tex);
    textureMap[Image::checkbox] = loadTextureFromFile(checkbox_tex);
    textureMap[Image::checkbox_checked] = loadTextureFromFile(checkbox_checked_tex);
    textureMap[Image::list_add] = loadTextureFromFile(list_add_tex);
    textureMap[Image::sub_add_next] = loadTextureFromFile(sub_add_next);
    textureMap[Image::sub_add_prev] = loadTextureFromFile(sub_add_prev);
    textureMap[Image::sub_remove_next] = loadTextureFromFile(sub_remove_next);
    textureMap[Image::sub_remove_prev] = loadTextureFromFile(sub_remove_prev);
    textureMap[Image::circle_fast_forward] = loadTextureFromFile(circle_fast_forward);
    textureMap[Image::circle_forward] = loadTextureFromFile(circle_forward);
    textureMap[Image::circle_play] = loadTextureFromFile(circle_play);
    textureMap[Image::circle_stop] = loadTextureFromFile(circle_stop);
    return textureMap;
}
} // namespace context
