#pragma once
#include "GlfwImguiContext.h"

#include <GL/gl.h>

#include <filesystem>
#include <map>
#include <memory>

namespace context {

enum class Image {
    cards,
};

struct TextureData;

class Texture
{
    static constexpr auto cards_tex = "/home/harmen/src/zikhron/resources/icons/cards_64px.png";

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
    int width{};
    int height{};
    GLuint data{};
};

} // namespace context
