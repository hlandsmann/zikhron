#pragma once
#include <utils/Property.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

struct SubText {
    const std::string text;
    const int64_t start_time;
    const int duration;
};

class Subtitle {
    friend class SubtitlesDecoder;
    std::vector<SubText> subtext;
    const AVCodecID codecID;

public:
    struct Init {
        const bool isTextSub;
        const std::string language;
        const std::string title;
        const AVCodecID codecID;
    };
    Subtitle(Init& init)
        : codecID(init.codecID), isTextSub(init.isTextSub), language(init.language), title(init.title){};
    auto cbegin() const { return subtext.cbegin(); }
    auto cend() const { return subtext.cend(); };
    auto empty() const { return subtext.empty(); };

    const bool isTextSub;
    const std::string language;
    const std::string title;
};

class SubtitlesDecoder {
public:
    SubtitlesDecoder(const std::string& filename);

private:
    std::function<void(AVCodecContext* context)> AVContextDeleter = [](AVCodecContext* context) {
        avcodec_free_context(&context);
    };
    using AVCodecContextPtr = std::unique_ptr<AVCodecContext, decltype(AVContextDeleter)>;
    std::map<AVCodecID, AVCodecContextPtr> id_avContext;

    auto create_AVCodecContext(AVCodecID codecID) -> AVCodecContextPtr;
    auto get_AVCodecContext(AVCodecID codecID) -> const AVCodecContextPtr&;
    auto decode_subtext(const AVSubtitle& av_subtitle) const -> std::string;

    std::map<int, Subtitle> subtitles;
    std::jthread worker;
};