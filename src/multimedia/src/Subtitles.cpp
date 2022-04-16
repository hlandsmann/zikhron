#include <Subtitles.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <functional>
#include <ranges>
#include <span>
namespace ranges = std::ranges;

SubtitlesDecoder::SubtitlesDecoder(const std::string& filename) {
    AVFormatContext* pFormatCtx = NULL;
    if (avformat_open_input(&pFormatCtx, filename.c_str(), nullptr, nullptr) != 0)
        spdlog::error("Could not open file '{}'", filename);
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
        spdlog::error("No Stream Info");
    spdlog::info("Duration: {}", pFormatCtx->duration);
    // AVCodecParameters* pCodecCtxOrig = NULL;
    // AVCodecParameters* pCodecCtx     = NULL;

    auto streams = std::span(pFormatCtx->streams, pFormatCtx->streams + pFormatCtx->nb_streams);
    std::vector<AVStream*> subtitleStreams;
    ranges::copy_if(streams, std::back_inserter(subtitleStreams), [](AVStream* stream) {
        return stream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE;
    });
    // // Find the first video stream
    // int videoStream = -1;
    // for (i = 0; i < pFormatCtx->nb_streams; i++)
    //     if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
    //         videoStream = i;
    //         break;
    //     }
    // if (videoStream == -1)
    //     spdlog::error("Could not find a subtitle stream");
    // spdlog::info("Streams found: {}", pFormatCtx->nb_streams);
    // // Get a pointer to the codec context for the video stream
    // pCodecCtx = pFormatCtx->streams[videoStream]->codecpar;
    for (AVStream* sub : subtitleStreams) {
        AVDictionaryEntry* sub_language = av_dict_get(
            sub->metadata, "language", nullptr, AV_DICT_IGNORE_SUFFIX);
        AVDictionaryEntry* sub_title = av_dict_get(
            sub->metadata, "title", nullptr, AV_DICT_IGNORE_SUFFIX);
        spdlog::info("Language: {}, {}",
                     sub_language ? sub_language->value : "-",
                     sub_title ? sub_title->value : "-");

        // AVDictionaryEntry* entry = NULL;
        // while (entry = av_dict_get(sub->metadata, "", entry, AV_DICT_IGNORE_SUFFIX)) {
        //     spdlog::info("Key: {} - value: {}", entry->key, entry->value);
        // }
        // auto dec_desc = avcodec_descriptor_get(sub->codecpar->codec_id);
        auto dec_desc = avcodec_descriptor_get(sub->codecpar->codec_id);
        if (dec_desc && !(dec_desc->props & AV_CODEC_PROP_TEXT_SUB)) {
            spdlog::error("Only text based subtitles are supported!");
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
    while (av_read_frame(pFormatCtx, &pkt) >= 0) {
        // if (pkt.stream_index != sub->index)
        //     continue;
        if (const auto it = subtitles.find(pkt.stream_index); it != subtitles.end()) {
            auto& subtitle = it->second;
            if (not subtitle.isTextSub)
                continue;

            // DoDo: Remove:
            if (subtitle.title != "Simplified")
                continue;

            const auto& codecCtx = get_AVCodecContext(subtitle.codecID);

            AVSubtitle av_subtitle;
            int gotSubtitle = 0;

            if (avcodec_decode_subtitle2(codecCtx.get(), &av_subtitle, &gotSubtitle, &pkt) < 0) {
                spdlog::error("Failed to decode subtitle");
                continue;
            }
            // (" {} ({} - {})", av_subtitle.rects[0]->ass, pkt.pts, pkt.duration);
            i++;
            if (percent < (100 * 1000 * pkt.pts) / pFormatCtx->duration) {
                percent = (100 * 1000 * pkt.pts) / pFormatCtx->duration;
                spdlog::warn("Precent: {}", percent);
            }
            subtitle.subtext.push_back(
                {.text = decode_subtext(av_subtitle), .start_time = pkt.pts, .duration = pkt.duration});
            avsubtitle_free(&av_subtitle);
        }
    }
    spdlog::info("Decoded {} frames.", i);
}

auto SubtitlesDecoder::create_AVCodecContext(AVCodecID codecID)
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

auto SubtitlesDecoder::get_AVCodecContext(AVCodecID codecID)
    -> const std::unique_ptr<AVCodecContext, decltype(AVContextDeleter)>& {
    if (not id_avContext.contains(codecID)) {
        id_avContext[codecID] = create_AVCodecContext(codecID);
    }
    return id_avContext.at(codecID);
}

auto SubtitlesDecoder::decode_subtext(const AVSubtitle& av_subtitle) const -> std::string {
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
