#pragma once
#include <filesystem>
#include <map>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>

struct AudioFragment {
    double start{};
    double end{};
};

struct StudyAudioFragment {
    double start{};
    double end{};
    std::filesystem::path audioFile{};
};

struct CardAudioGroup {
    std::filesystem::path audioFile;
    std::map<uint, AudioFragment> cardId_audioFragment;

    static auto toJson(const CardAudioGroup &) -> nlohmann::json;
    static auto fromJson(const nlohmann::json &) -> CardAudioGroup;

    bool onFileSystem = false;

private:
    static constexpr std::string_view s_audioFile = "audio_file";
    static constexpr std::string_view s_fragments = "fragments";
    static constexpr std::string_view s_start = "start";
    static constexpr std::string_view s_end = "end";
};

class CardAudioGroupDB {
    static constexpr uint s_firstGroupId = 1;
    static constexpr uint s_invalidGroupId = 0;
    static constexpr std::string_view s_groupDir = "/home/harmen/zikhron/group/";
    CardAudioGroupDB();

public:
    static auto get() -> CardAudioGroupDB &;
    void save(uint groupId, const CardAudioGroup &);
    void insert(uint groupId, const CardAudioGroup &);
    [[nodiscard]] auto seekBackward(uint cardId) const -> std::optional<uint>;
    [[nodiscard]] auto seekForward(uint cardId) const -> std::optional<uint>;
    [[nodiscard]] auto skipBackward(uint cardId) const -> std::optional<uint>;
    [[nodiscard]] auto skipForward(uint cardId) const -> std::optional<uint>;
    [[nodiscard]] auto nextOrThisGroupId(uint groupId) const -> uint;
    [[nodiscard]] auto prevOrThisGroupId(uint groupId) const -> uint;
    [[nodiscard]] auto newCardAudioGroup() -> uint;
    [[nodiscard]] auto get_cardAudioGroup(uint groupId) const -> CardAudioGroup;
    [[nodiscard]] auto get_studyAudioFragment(uint cardId) const -> std::optional<StudyAudioFragment>;

private:
    void load();
    void setupStudyAudioFragments();
    [[nodiscard]] auto findAudioGroupFromCardId(uint cardId) const -> std::optional<uint>;

    std::map<uint, CardAudioGroup> id_cardAudioGroup;
    std::map<uint, StudyAudioFragment> cardId_studyAudioFragment;

    [[nodiscard]] auto cardIdIt_and_group(uint cardId) const -> std::optional<
        std::pair<decltype(std::ranges::begin(id_cardAudioGroup.begin()->second.cardId_audioFragment)),
                  const decltype(CardAudioGroup().cardId_audioFragment) &>>;
};
