#include <Subtitles.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <functional>
#include <gsl/util>
#include <ranges>
#include <span>

namespace ranges = std::ranges;

SubtitleDecoder::SubtitleDecoder(const std::string& _filename) : filename(_filename) {
    worker = std::jthread([this](std::stop_token token) { worker_thread(token); });
}

SubtitleDecoder::~SubtitleDecoder() {
    worker.request_stop();
    worker.join();
}

// auto SubtitleDecoder::observeProgress(const std::function<void(double)>& observeCallback)
//     -> std::shared_ptr<utl::Observer<double>> {
//     return progress.observe(observeCallback);
// }
// auto SubtitleDecoder::observeFinished(const std::function<void(bool)>& finishedCallback)
//     -> std::shared_ptr<utl::Observer<bool>> {
//     return finished.observe(finishedCallback);
// }

void SubtitleDecoder::worker_thread(std::stop_token token) {
    AVFormatContext* pFormatCtx = NULL;
    if (avformat_open_input(&pFormatCtx, filename.c_str(), nullptr, nullptr) != 0) {
        spdlog::error("Could not open file '{}'", filename);
        return;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        spdlog::error("No Stream Info");
        return;
    }
    spdlog::info("Duration: {}", pFormatCtx->duration);

    auto streams = std::span(pFormatCtx->streams, pFormatCtx->streams + pFormatCtx->nb_streams);
    std::vector<AVStream*> subtitleStreams;
    ranges::copy_if(streams, std::back_inserter(subtitleStreams), [](AVStream* stream) {
        return stream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE;
    });

    for (AVStream* sub : subtitleStreams) {
        AVDictionaryEntry* sub_language = av_dict_get(
            sub->metadata, "language", nullptr, AV_DICT_IGNORE_SUFFIX);
        AVDictionaryEntry* sub_title = av_dict_get(
            sub->metadata, "title", nullptr, AV_DICT_IGNORE_SUFFIX);
        spdlog::info("Language: {}, {}",
                     sub_language ? sub_language->value : "-",
                     sub_title ? sub_title->value : "-");

        auto dec_desc = avcodec_descriptor_get(sub->codecpar->codec_id);
        if (dec_desc && !(dec_desc->props & AV_CODEC_PROP_TEXT_SUB)) {
            spdlog::warn("Only text based subtitles are supported!");
        }
        Subtitle::Init subInit = {.isTextSub = 0 != (dec_desc->props & AV_CODEC_PROP_TEXT_SUB),
                                  .language = sub_language ? sub_language->value : "-",
                                  .title = sub_title ? sub_title->value : "-",
                                  .codecID = sub->codecpar->codec_id};
        subtitles.emplace(sub->index, subInit);
    }

    AVPacket pkt;
    int i = 0;
    int percent = -1;
    while (av_read_frame(pFormatCtx, &pkt) >= 0 && !token.stop_requested()) {
        auto finally = gsl::final_action([&pkt]() { av_packet_unref(&pkt); });

        if (const auto it = subtitles.find(pkt.stream_index); it != subtitles.end()) {
            auto& subtitle = it->second;
            if (not subtitle.isTextSub)
                continue;

            const auto& codecCtx = get_AVCodecContext(subtitle.codecID);

            AVSubtitle av_subtitle;
            int gotSubtitle = 0;

            if (avcodec_decode_subtitle2(codecCtx.get(), &av_subtitle, &gotSubtitle, &pkt) < 0) {
                spdlog::error("Failed to decode subtitle");
                continue;
            }
            i++;
            if (percent < (100 * 1000 * pkt.pts) / pFormatCtx->duration) {
                progress = (1000. * double(pkt.pts)) / double(pFormatCtx->duration);
            }
            subtitle.subtext.push_back(
                {.text = decode_subtext(av_subtitle), .start_time = pkt.pts, .duration = pkt.duration});
            avsubtitle_free(&av_subtitle);
        }
    }
    if (token.stop_requested()) {
        spdlog::info("Cancel subtitle decoding..");
        return;
    }
    spdlog::info("Decoded {} frames.", i);
    finished = true;
}

auto SubtitleDecoder::create_AVCodecContext(AVCodecID codecID)
    -> std::unique_ptr<AVCodecContext, decltype(AVContextDeleter)> {
    auto codec = avcodec_find_decoder(codecID);
    spdlog::info("Decoder: {}", avcodec_get_name(codecID));
    if (!codec) {
        spdlog::error("Failed to find decoder for {}", avcodec_get_name(codecID));
    }
    auto codecCtx = avcodec_alloc_context3(codec);
    if (not codecCtx) {
        spdlog::error("Failed to alloc codec context for {}", avcodec_get_name(codecID));
    }
    if (avcodec_open2(codecCtx, codec, NULL)) {
        spdlog::error("Failed to open decoder for {}", avcodec_get_name(codecID));
    }
    return AVCodecContextPtr(codecCtx, AVContextDeleter);
}

auto SubtitleDecoder::get_AVCodecContext(AVCodecID codecID)
    -> const std::unique_ptr<AVCodecContext, decltype(AVContextDeleter)>& {
    if (not id_avContext.contains(codecID)) {
        id_avContext[codecID] = create_AVCodecContext(codecID);
    }
    return id_avContext.at(codecID);
}

auto SubtitleDecoder::decode_subtext(const AVSubtitle& av_subtitle) const -> std::string {
    auto rects = std::span(av_subtitle.rects, av_subtitle.rects + av_subtitle.num_rects);
    std::string subtext;
    for (const auto& rect : rects) {
        constexpr int ass_segments = 9;
        std::string_view ass = rect->ass;
        std::string_view parsed = ass;
        if (std::string_view::npos == ass.find("Dialogue"))
            continue;
        for (int i = 0; i < ass_segments; i++) {
            std::size_t commaPos = parsed.find(',');
            if (commaPos == std::string_view::npos) {
                spdlog::error("ass subtitle parsing error for {}", ass);
                break;
            }
            parsed = parsed.substr(commaPos + 1);
        }
        subtext += parsed;
    }
    return subtext;
}
