#pragma once
#include <map>
#include <GL/gl.h>

#include <filesystem>

#include <sys/types.h>
namespace context {

enum class Image {
    cards,
};

struct TextureData;

class Texture
{
    static constexpr auto cards_tex = "/home/harmen/src/zikhron/resources/icons/cards_64px.png";

public:
    Texture();
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
