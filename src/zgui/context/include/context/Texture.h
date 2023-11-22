#pragma once
#include "GlfwImguiContext.h"

#include <GL/gl.h>

#include <filesystem>
#include <map>
#include <memory>

namespace context {

enum class Image {
    cards,
    audio,
    configure,
    video,
};

struct TextureData;

class Texture
{
    static constexpr auto audio_tex = "/home/harmen/src/zikhron/resources/icons/audio_48px.png";
    static constexpr auto cards_tex = "/home/harmen/src/zikhron/resources/icons/cards_48px.png";
    static constexpr auto configure_tex = "/home/harmen/src/zikhron/resources/icons/configure_48px.png";
    static constexpr auto video_tex = "/home/harmen/src/zikhron/resources/icons/video_48px.png";

public:
    // the GlfwImguiContext needs to be initialized before this class is constructed
    Texture(std::shared_ptr<GlfwImguiContext> /* glfwImguiContext */);
    [[nodiscard]] auto get(Image image) const -> const TextureData&;

private:
    static auto loadTextureFromFile(const std::filesystem::path& imageFile) -> TextureData;
    static auto loadTextureMap() -> std::map<Image, TextureData>;

    std::map<Image, TextureData> textureMap;
};

struct TextureData
{
    float width{};
    float height{};
    GLuint data{};
};

} // namespace context
