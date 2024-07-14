#include <ExtractSubtitles.h>
#include <spdlog/spdlog.h>
#include <utils/format.h>
#include <utils/string_split.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <gsl/util>
#include <iterator>
#include <magic_enum.hpp>
#include <map>
#include <memory>
#include <ranges>
#include <span>
#include <stop_token>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavcodec/codec_desc.h>
#include <libavcodec/codec_id.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>
}
namespace ranges = std::ranges;

namespace {

struct InternalSub
{
    std::vector<multimedia::SubText> subText;
    AVCodecID codecID;
    std::string language;
    std::string title;
    int index;
};

struct AVContextDeleter
{
    void operator()(AVCodecContext* context)
    {
        avcodec_free_context(&context);
    }
};

using AVCodecContextPtr = std::unique_ptr<AVCodecContext, AVContextDeleter>;

auto create_AVCodecContext(AVCodecID codecID)
        -> AVCodecContextPtr
{
    const auto* codec = avcodec_find_decoder(codecID);
    spdlog::info("Decoder: {}", avcodec_get_name(codecID));
    if (codec == nullptr) {
        spdlog::error("Failed to find decoder for {}", avcodec_get_name(codecID));
    }
    auto* codecCtx = avcodec_alloc_context3(codec);
    if (codecCtx == nullptr) {
        spdlog::error("Failed to alloc codec context for {}", avcodec_get_name(codecID));
    }
    if (avcodec_open2(codecCtx, codec, nullptr) != 0) {
        spdlog::error("Failed to open decoder for {}", avcodec_get_name(codecID));
    }
    return {codecCtx, AVContextDeleter{}};
}

auto get_AVCodecContext(AVCodecID codecID, std::map<AVCodecID, AVCodecContextPtr>& id_avContext)
        -> const AVCodecContextPtr&
{
    if (not id_avContext.contains(codecID)) {
        id_avContext[codecID] = create_AVCodecContext(codecID);
    }
    return id_avContext.at(codecID);
}

auto decodeSubtext(const AVSubtitle& av_subtitle, int64_t startTime, int64_t duration) -> multimedia::SubText
{
    auto rects = std::span(av_subtitle.rects, std::next(av_subtitle.rects, av_subtitle.num_rects));
    std::string ass;
    for (const auto& rect : rects) {
        ass += rect->ass;
    }
    // there should be 10 ass_segments, but ffmpeg only gets 9???
    // examples for fmt::print("{}\n", ass) at the time of writing...
    // `819,1,Dial-CH,,0,0,0,,就交给小春吧`
    // `372,0,Dial-JP,,0,0,0,,最初は断った人間が`
    // `6165,0,Default,,0,0,0,,告诉我 会实现吧`

    auto rest = std::string_view{ass};

    /* dialogIndex */ utl::split_front(rest, ",");
    /* alt unkown  */ utl::split_front(rest, ",");
    auto style = utl::split_front(rest, ",");
    /*   unkown   */ utl::split_front(rest, ",");
    /*   unkown   */ utl::split_front(rest, ",");
    /*   unkown   */ utl::split_front(rest, ",");
    /*   unkown   */ utl::split_front(rest, ",");
    /*   unkown   */ utl::split_front(rest, ",");
    auto text = rest;
    if (text.empty()) {
        spdlog::error("decodeSubtext: empty text segment");
    }
    // fmt::print(" {},  {}\n", style, text);
    return {.startTime = startTime,
            .duration = duration,
            .style = std::string{style},
            .text = std::string{text}};
}

auto findSubtitles(AVFormatContext* pFormatCtx) -> std::map<int, InternalSub>
{
    std::map<int, InternalSub> subtitles;

    auto streams = std::span(pFormatCtx->streams, std::next(pFormatCtx->streams, pFormatCtx->nb_streams));
    std::vector<AVStream*> subtitleStreams;
    ranges::copy_if(streams, std::back_inserter(subtitleStreams), [](AVStream* stream) {
        return stream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE;
    });

    for (AVStream* sub : subtitleStreams) {
        AVDictionaryEntry* sub_language = av_dict_get(
                sub->metadata, "language", nullptr, AV_DICT_IGNORE_SUFFIX);
        AVDictionaryEntry* sub_title = av_dict_get(
                sub->metadata, "title", nullptr, AV_DICT_IGNORE_SUFFIX);
        std::string language = sub_language != nullptr
                                       ? sub_language->value
                                       : "";
        std::string title = sub_title != nullptr
                                    ? sub_title->value
                                    : "";

        const auto* const dec_desc = avcodec_descriptor_get(sub->codecpar->codec_id);
        if (dec_desc != nullptr && (0 == (dec_desc->props & AV_CODEC_PROP_TEXT_SUB))) {
            spdlog::warn("Subtitle index: {}, lang: {}, title: {}.. Only text based subtitles are supported!",
                         sub->index, language, title);
            continue;
        }

        InternalSub subInit = {.subText = {},
                               .codecID = sub->codecpar->codec_id,
                               .language = language,
                               .title = title,
                               .index = sub->index};
        subtitles.emplace(sub->index, subInit);
    }
    return subtitles;
}

void insertSubtitleFrame(std::map<int, InternalSub>& subtitles,
                         const AVPacket& pkt,
                         std::map<AVCodecID, AVCodecContextPtr>& id_avContext)
{
    const auto it = subtitles.find(pkt.stream_index);
    if (it == subtitles.end()) {
        return;
    }
    auto& subtitle = it->second;

    const auto& codecCtx = get_AVCodecContext(subtitle.codecID, id_avContext);

    AVSubtitle av_subtitle;
    int gotSubtitle = 0;

    if (avcodec_decode_subtitle2(codecCtx.get(), &av_subtitle, &gotSubtitle, &pkt) < 0) {
        spdlog::error("Failed to decode subtitle");
        return;
    }
    auto subText = decodeSubtext(av_subtitle, pkt.pts, pkt.duration);
    if (!subText.text.empty()) {
        subtitle.subText.push_back(subText);
    }
    avsubtitle_free(&av_subtitle);
}
} // namespace

namespace multimedia {
ExtractSubtitles::ExtractSubtitles(std::filesystem::path _videoFile)
    : filename{std::move(_videoFile)}
{
}

auto convertInternalSubs(std::map<int, InternalSub>&& internalSubs) -> std::vector<Subtitle>
{
    std::vector<Subtitle> subtitles;
    ranges::transform(internalSubs, std::back_inserter(subtitles),
                      [](auto&& idxSub) -> Subtitle {
                          const auto& [_, internalSub] = idxSub;
                          auto subtitle = Subtitle{.language = internalSub.language,
                                                   .title = internalSub.title,
                                                   .indexInVideo = internalSub.index,
                                                   .subs = std::move(internalSub.subText)};

                          return subtitle;
                      });

    return subtitles;
}

auto ExtractSubtitles::decode(std::stop_token token) -> std::vector<Subtitle>
{
    std::map<int, InternalSub> internalSubs;
    std::map<AVCodecID, AVCodecContextPtr> id_avContext;
    AVFormatContext* pFormatCtx = nullptr;
    if (avformat_open_input(&pFormatCtx, filename.c_str(), nullptr, nullptr) != 0) {
        spdlog::error("Could not open file '{}'", filename);
        return {};
    }
    auto finally = gsl::finally([&pFormatCtx]() {
        avformat_close_input(&pFormatCtx);
    });
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        spdlog::error("No Stream Info");
        return {};
    }
    spdlog::info("Duration: {}", pFormatCtx->duration);
    internalSubs = findSubtitles(pFormatCtx);
    if (internalSubs.empty()) {
        spdlog::warn("No subtitles found in '{}'", filename);
        return {};
    }

    AVPacket pkt;
    while (av_read_frame(pFormatCtx, &pkt) >= 0 && !token.stop_requested()) {
        progress = (1000. * double(pkt.pts)) / double(pFormatCtx->duration);
        insertSubtitleFrame(internalSubs, pkt, id_avContext);
        av_packet_unref(&pkt);
    }
    if (token.stop_requested()) {
        spdlog::info("Cancel subtitle decoding..");
        return {};
    }
    for (const auto& [_, sub] : internalSubs) {
        spdlog::info("sub {}:, lang: {}, title: {}, size: {}", sub.index, sub.language, sub.title, sub.subText.size());
    }
    progress = 1.0;
    return convertInternalSubs(std::move(internalSubs));
}

auto ExtractSubtitles::getProgress() const -> double
{
    return progress;
}

} // namespace multimedia
