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
    configure_app,
    configure_mid,
    configure,
    video,
    document_open,
    document_save,
    media_continue,
    media_playback_pause,
    media_playback_start,
    media_playback_stop,
    media_seek_backward,
    media_seek_forward,
    media_skip_backward,
    media_skip_forward,
    new_audio_alarm,
    arrow_up,
    arrow_down,
    arrow_left,
    arrow_right,
    checkbox,
    checkbox_checked,
    list_add,
    sub_add_next,
    sub_add_prev,
    sub_cut_next,
    sub_cut_prev,
    circle_fast_forward,
    circle_forward,
    circle_play,
    circle_stop,
    object_select,
    object_unselect,
};

struct TextureData;

class Texture
{
    static constexpr auto audio = "/home/harmen/src/zikhron/build/resources/icons/audiobook_48px.png";
    static constexpr auto cards = "/home/harmen/src/zikhron/build/resources/icons/cards_48px.png";
    static constexpr auto configure_app = "/home/harmen/src/zikhron/build/resources/icons/configure_app_48px.png";
    static constexpr auto configure = "/home/harmen/src/zikhron/build/resources/icons/configure_24px.png";
    static constexpr auto configure_mid = "/home/harmen/src/zikhron/build/resources/icons/configure_32px.png";
    static constexpr auto video = "/home/harmen/src/zikhron/build/resources/icons/video_48px.png";
    static constexpr auto document_open = "/home/harmen/src/zikhron/build/resources/icons/document-open_32px.png";
    static constexpr auto document_save = "/home/harmen/src/zikhron/build/resources/icons/document-save_32px.png";
    static constexpr auto media_continue = "/home/harmen/src/zikhron/build/resources/icons/continue_32px.png";
    static constexpr auto media_playback_pause = "/home/harmen/src/zikhron/build/resources/icons/media-playback-pause_32px.png";
    static constexpr auto media_playback_start = "/home/harmen/src/zikhron/build/resources/icons/media-playback-start_32px.png";
    static constexpr auto media_playback_stop = "/home/harmen/src/zikhron/build/resources/icons/media-playback-stop_32px.png";
    static constexpr auto media_seek_backward = "/home/harmen/src/zikhron/build/resources/icons/media-seek-backward_32px.png";
    static constexpr auto media_seek_forward = "/home/harmen/src/zikhron/build/resources/icons/media-seek-forward_32px.png";
    static constexpr auto media_skip_backward = "/home/harmen/src/zikhron/build/resources/icons/media-skip-backward_32px.png";
    static constexpr auto media_skip_forward = "/home/harmen/src/zikhron/build/resources/icons/media-skip-forward_32px.png";
    static constexpr auto new_audio_alarm = "/home/harmen/src/zikhron/build/resources/icons/new-audio-alarm_32px.png";
    static constexpr auto arrow_up = "/home/harmen/src/zikhron/build/resources/icons/pan-up_24px.png";
    static constexpr auto arrow_down = "/home/harmen/src/zikhron/build/resources/icons/pan-down_24px.png";
    static constexpr auto arrow_left = "/home/harmen/src/zikhron/build/resources/icons/pan-start_24px.png";
    static constexpr auto arrow_right = "/home/harmen/src/zikhron/build/resources/icons/pan-end_24px.png";
    static constexpr auto checkbox = "/home/harmen/src/zikhron/build/resources/icons/checkbox-symbolic_24px.png";
    static constexpr auto checkbox_checked = "/home/harmen/src/zikhron/build/resources/icons/checkbox-checked-symbolic_24px.png";
    static constexpr auto list_add = "/home/harmen/src/zikhron/build/resources/icons/list-add_24px.png";
    static constexpr auto sub_add_next = "/home/harmen/src/zikhron/build/resources/icons/sub-add-next_32px.png";
    static constexpr auto sub_add_prev = "/home/harmen/src/zikhron/build/resources/icons/sub-add-prev_32px.png";
    static constexpr auto sub_cut_next = "/home/harmen/src/zikhron/build/resources/icons/sub-cut-next_32px.png";
    static constexpr auto sub_cut_prev = "/home/harmen/src/zikhron/build/resources/icons/sub-cut-prev_32px.png";
    static constexpr auto circle_fast_forward = "/home/harmen/src/zikhron/build/resources/icons/circle-fast-forward_32px.png";
    static constexpr auto circle_forward = "/home/harmen/src/zikhron/build/resources/icons/circle-forward_32px.png";
    static constexpr auto circle_play = "/home/harmen/src/zikhron/build/resources/icons/circle-play_32px.png";
    static constexpr auto circle_stop = "/home/harmen/src/zikhron/build/resources/icons/circle-stop_32px.png";
    static constexpr auto object_select = "/home/harmen/src/zikhron/build/resources/icons/object-select_32px.png";
    static constexpr auto object_unselect = "/home/harmen/src/zikhron/build/resources/icons/object-unselect_32px.png";

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
