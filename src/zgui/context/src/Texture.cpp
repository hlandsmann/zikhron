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
    textureMap[Image::audio] = loadTextureFromFile(audio);
    textureMap[Image::cards] = loadTextureFromFile(cards);
    textureMap[Image::configure_app] = loadTextureFromFile(configure_app);
    textureMap[Image::configure_mid] = loadTextureFromFile(configure_mid);
    textureMap[Image::configure] = loadTextureFromFile(configure);
    textureMap[Image::video] = loadTextureFromFile(video);
    textureMap[Image::document_open] = loadTextureFromFile(document_open);
    textureMap[Image::document_save] = loadTextureFromFile(document_save);
    textureMap[Image::media_continue] = loadTextureFromFile(media_continue);
    textureMap[Image::media_playback_pause] = loadTextureFromFile(media_playback_pause);
    textureMap[Image::media_playback_start] = loadTextureFromFile(media_playback_start);
    textureMap[Image::media_playback_stop] = loadTextureFromFile(media_playback_stop);
    textureMap[Image::media_seek_backward] = loadTextureFromFile(media_seek_backward);
    textureMap[Image::media_seek_forward] = loadTextureFromFile(media_seek_forward);
    textureMap[Image::media_skip_backward] = loadTextureFromFile(media_skip_backward);
    textureMap[Image::media_skip_forward] = loadTextureFromFile(media_skip_forward);
    textureMap[Image::new_audio_alarm] = loadTextureFromFile(new_audio_alarm);
    textureMap[Image::arrow_up] = loadTextureFromFile(arrow_up);
    textureMap[Image::arrow_down] = loadTextureFromFile(arrow_down);
    textureMap[Image::arrow_left] = loadTextureFromFile(arrow_left);
    textureMap[Image::arrow_right] = loadTextureFromFile(arrow_right);
    textureMap[Image::checkbox] = loadTextureFromFile(checkbox);
    textureMap[Image::checkbox_checked] = loadTextureFromFile(checkbox_checked);
    textureMap[Image::list_add] = loadTextureFromFile(list_add);
    textureMap[Image::sub_add_next] = loadTextureFromFile(sub_add_next);
    textureMap[Image::sub_add_prev] = loadTextureFromFile(sub_add_prev);
    textureMap[Image::sub_cut_next] = loadTextureFromFile(sub_cut_next);
    textureMap[Image::sub_cut_prev] = loadTextureFromFile(sub_cut_prev);
    textureMap[Image::circle_fast_forward] = loadTextureFromFile(circle_fast_forward);
    textureMap[Image::circle_forward] = loadTextureFromFile(circle_forward);
    textureMap[Image::circle_play] = loadTextureFromFile(circle_play);
    textureMap[Image::circle_stop] = loadTextureFromFile(circle_stop);
    textureMap[Image::object_select] = loadTextureFromFile(object_select);
    textureMap[Image::object_unselect] = loadTextureFromFile(object_unselect);
    textureMap[Image::translation] = loadTextureFromFile(translation);
    textureMap[Image::time_delete_backward] = loadTextureFromFile(time_delete_backward);
    textureMap[Image::time_delete_forward] = loadTextureFromFile(time_delete_forward);
    textureMap[Image::time_backward] = loadTextureFromFile(time_backward);
    textureMap[Image::time_forward] = loadTextureFromFile(time_forward);
    textureMap[Image::time_subtitle] = loadTextureFromFile(time_subtitle);
    textureMap[Image::flag_brazil] = loadTextureFromFile(flag_brazil);
    textureMap[Image::flag_china] = loadTextureFromFile(flag_china);
    textureMap[Image::flag_israel] = loadTextureFromFile(flag_israel);
    textureMap[Image::flag_japan] = loadTextureFromFile(flag_japan);
    textureMap[Image::flag_russia] = loadTextureFromFile(flag_russia);
    textureMap[Image::flag_spain] = loadTextureFromFile(flag_spain);
    textureMap[Image::flag_uk] = loadTextureFromFile(flag_uk);
    return textureMap;
}
} // namespace context
