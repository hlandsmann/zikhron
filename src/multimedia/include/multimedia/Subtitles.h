#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

struct SubText
{
    const std::string text;
    const int64_t start_time;
    const int64_t duration;
};

class Subtitle
{
    friend class SubtitleDecoder;
    std::vector<SubText> subtext;
    const AVCodecID codecID;

public:
    struct Init
    {
        const bool isTextSub;
        const std::string language;
        const std::string title;
        const AVCodecID codecID;
    };

    Subtitle(Init& init)
        : codecID(init.codecID), isTextSub(init.isTextSub), language(init.language), title(init.title) {};

    auto cbegin() const { return subtext.cbegin(); }

    auto cend() const { return subtext.cend(); };

    auto empty() const { return subtext.empty(); };

    const bool isTextSub;
    const std::string language;
    const std::string title;
};

class SubtitleDecoder
{
public:
    SubtitleDecoder(const std::string& filename);
    ~SubtitleDecoder();

    // auto observeProgress(const std::function<void(double)>&) -> std::shared_ptr<utl::Observer<double>>;
    // auto observeFinished(const std::function<void(bool)>&) -> std::shared_ptr<utl::Observer<bool>>;

private:
    void worker_thread(std::stop_token token);
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
    std::string filename;
    double progress = 0.;
    bool finished = false;
};
