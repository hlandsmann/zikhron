#pragma once
#include <filesystem>
#include <map>
#include <nlohmann/json_fwd.hpp>
#include <string_view>

struct AudioFragment {
    double start;
    double end;
};

struct CardAudioGroup {
    std::filesystem::path audioFile;
    std::map<uint, AudioFragment> cardId_audioFragment;

    static auto toJson(const CardAudioGroup &) -> nlohmann::json;
    static auto fromJson(const nlohmann::json &) -> CardAudioGroup;

private:
    static constexpr std::string_view s_audioFile = "audio_file";
    static constexpr std::string_view s_fragments = "fragments";
    static constexpr std::string_view s_start = "start";
    static constexpr std::string_view s_end = "end";
};

class CardAudioGroupDB {
    static constexpr std::string_view s_groupDir = "/home/harmen/zikhron/group/";
    CardAudioGroupDB();

public:
    static auto get() -> CardAudioGroupDB &;
    void save(uint groupId, const CardAudioGroup &);
    auto get_cardAudioGroup(uint groupId) const -> CardAudioGroup;

private:
    void load();
    std::map<uint, CardAudioGroup> id_cardAudioGroup;
};