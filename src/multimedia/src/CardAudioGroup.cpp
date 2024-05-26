#include <CardAudioGroup.h>
#include <fmt/format.h>
#include <misc/Identifier.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <ctre.hpp>
#include <ctre/wrapper.hpp>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <iterator>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <sys/types.h>

namespace ranges = std::ranges;
namespace fs = std::filesystem;
using json = nlohmann::json;

auto CardAudioGroup::toJson(const CardAudioGroup& self) -> json
{
    json cardAudioGroup = json::object();
    cardAudioGroup[std::string(s_audioFile)] = self.audioFile;

    json json_cardId_fragment = json::object();
    for (const auto& [cardId, audioFragment] : self.cardId_audioFragment) {
        json jsonFragment = json::object();
        jsonFragment[s_start] = audioFragment.start;
        jsonFragment[s_end] = audioFragment.end;
        json_cardId_fragment[std::to_string(cardId)] = jsonFragment;
    }

    cardAudioGroup[std::string(s_fragments)] = json_cardId_fragment;
    return cardAudioGroup;
}

auto CardAudioGroup::fromJson(const json& cag_json) -> CardAudioGroup
{
    CardAudioGroup cardAudioGroup;
    auto& cardId_audioFragment = cardAudioGroup.cardId_audioFragment;
    cardAudioGroup.audioFile = std::string(cag_json[std::string(s_audioFile)]);
    ranges::transform(cag_json[std::string(s_fragments)].items(),
                      std::inserter(cardId_audioFragment, cardId_audioFragment.begin()),
                      [](const auto& cardId_fragment) -> std::pair<CardId, AudioFragment> {
                          const auto& [cardId, fragment_json] = cardId_fragment;
                          AudioFragment fragment = {.start = fragment_json[s_start],
                                                    .end = fragment_json[s_end]};
                          return {static_cast<CardId>(std::stoi(cardId)), fragment};
                      });
    return cardAudioGroup;
}

CardAudioGroupDB::CardAudioGroupDB()
{
    load();
}

void CardAudioGroupDB::load()
{
    fs::path path_dir_cardAudioGroup = s_groupDir;
    if (!fs::exists(path_dir_cardAudioGroup)) {
        return;
    }

    auto fn_matcher = ctre::match<"(\\d{6})(\\.group)">;
    for (const fs::path& entry : fs::directory_iterator(path_dir_cardAudioGroup)) {
        std::string fn = entry.filename().string();
        auto fn_match = fn_matcher(fn);
        if (not fn_match) {
            spdlog::warn("File '{}' in '{}' ignored. Please remove!", fn, path_dir_cardAudioGroup.string());
            continue;
        }
        try {
            uint groupId = static_cast<uint>(std::stoul(fn_match.get<1>().to_string()));
            std::ifstream groupFile(entry, std::ios::in | std::ios::binary);
            if (!groupFile) {
                throw std::runtime_error("Failure to read file");
            }
            if (id_cardAudioGroup.contains(groupId)) {
                spdlog::warn(
                        "GroupId {} ignored, because its already in use! Fn: {}", groupId, entry.string());
                continue;
            }
            id_cardAudioGroup[groupId] = CardAudioGroup::fromJson(json::parse(groupFile));
        } catch (std::exception& e) {
            spdlog::error("{} - file: {}", e.what(), entry.filename().string());
        }
    }
    setupStudyAudioFragments();
}

void CardAudioGroupDB::save(uint groupId, const CardAudioGroup& cardAudioGroup)
{
    id_cardAudioGroup[groupId] = cardAudioGroup;

    fs::path path_dir_cardAudioGroup = s_groupDir;
    try {
        if (!fs::exists(path_dir_cardAudioGroup)) {
            fs::create_directory(path_dir_cardAudioGroup);
        }

        fs::path fn_cardAudioGroup = path_dir_cardAudioGroup / std::format("{:06d}.group", groupId);
        std::ofstream ofs(fn_cardAudioGroup);
        ofs << CardAudioGroup::toJson(cardAudioGroup).dump(4);
        spdlog::debug("Saved {}", fn_cardAudioGroup.string());
        id_cardAudioGroup[groupId].onFileSystem = true;
        setupStudyAudioFragments();
    } catch (const std::exception& e) {
        spdlog::error("Failed to save file, exception: {}", e.what());
    }
}

auto CardAudioGroupDB::get_cardAudioGroup(uint groupId) const -> CardAudioGroup
{
    if (id_cardAudioGroup.contains(groupId)) {
        return id_cardAudioGroup.at(groupId);
    }
    return {};
}

void CardAudioGroupDB::insert(uint groupId, const CardAudioGroup& cag)
{
    if (groupId == s_invalidGroupId) {
        return;
    }

    spdlog::debug("group: {}, size {}", groupId, cag.cardId_audioFragment.size());
    bool is_saved = id_cardAudioGroup[groupId].onFileSystem;
    id_cardAudioGroup[groupId] = cag;
    id_cardAudioGroup[groupId].onFileSystem = is_saved;
    setupStudyAudioFragments();
}

auto CardAudioGroupDB::nextOrThisGroupId(uint groupId) const -> uint
{
    if (id_cardAudioGroup.empty()) {
        return s_firstGroupId;
    }
    auto it = ranges::find_if(
            id_cardAudioGroup,
            [groupId](uint nextId) { return nextId >= groupId; },
            &decltype(id_cardAudioGroup)::value_type::first);
    if (it == id_cardAudioGroup.end()) {
        return id_cardAudioGroup.begin()->first;
    }
    return it->first;
}

auto CardAudioGroupDB::prevOrThisGroupId(uint groupId) const -> uint
{
    if (id_cardAudioGroup.empty()) {
        return s_firstGroupId;
    }
    auto it = ranges::find_if(
            id_cardAudioGroup | std::views::reverse,
            [groupId](uint nextId) { return nextId <= groupId; },
            &decltype(id_cardAudioGroup)::value_type::first);
    if (it == id_cardAudioGroup.rend()) {
        return id_cardAudioGroup.rbegin()->first;
    }
    return it->first;
}

auto CardAudioGroupDB::newCardAudioGroup() -> uint
{
    uint newGroupId = s_firstGroupId;
    if (not id_cardAudioGroup.empty()) {
        if (auto it = ranges::adjacent_find(
                    id_cardAudioGroup, [](const auto& a, const auto& b) { return b.first - a.first != 1; });
            it != id_cardAudioGroup.end()) {
            newGroupId = it->first + 1;
        } else {
            newGroupId = id_cardAudioGroup.rbegin()->first + 1;
        }
    }
    spdlog::warn("new groupID: {}", newGroupId);
    id_cardAudioGroup[newGroupId] = CardAudioGroup{};
    return newGroupId;
}

void CardAudioGroupDB::setupStudyAudioFragments()
{
    for (const auto& cag : id_cardAudioGroup | std::views::values) {
        ranges::transform(
                cag.cardId_audioFragment,
                std::inserter(cardId_studyAudioFragment, cardId_studyAudioFragment.begin()),
                [&cag](const auto& cardId_fragment) -> std::pair<uint, StudyAudioFragment> {
                    const auto& [cardId, fragment] = cardId_fragment;
                    return {cardId,
                            {.start = fragment.start, .end = fragment.end, .audioFile = cag.audioFile}};
                });
    }
}

auto CardAudioGroupDB::get_studyAudioFragment(uint cardId) const -> std::optional<StudyAudioFragment>
{
    if (not cardId_studyAudioFragment.contains(cardId)) {
        return {};
    }
    return cardId_studyAudioFragment.at(cardId);
}

auto CardAudioGroupDB::seekForward(CardId cardId) const -> std::optional<CardId>
{
    const auto cardIdIt_group = cardIdIt_and_group(cardId);
    if (!cardIdIt_group.has_value()) {
        return {};
    }
    const auto& [cardId_It, cardId_audioFragment] = *cardIdIt_group;
    if (const auto next_cardId_It = std::next(cardId_It); next_cardId_It != cardId_audioFragment.end()) {
        return {next_cardId_It->first};
    }
    return {};
}

auto CardAudioGroupDB::seekBackward(CardId cardId) const -> std::optional<CardId>
{
    const auto cardIdIt_group = cardIdIt_and_group(cardId);
    if (!cardIdIt_group.has_value()) {
        return {};
    }
    const auto& [cardId_It, cardId_audioFragment] = *cardIdIt_group;
    if (cardId_It != cardId_audioFragment.begin()) {
        return {std::prev(cardId_It)->first};
    }
    return {};
}

auto CardAudioGroupDB::skipForward(CardId cardId) const -> std::optional<CardId>
{
    const auto cardIdIt_group = cardIdIt_and_group(cardId);
    if (!cardIdIt_group.has_value()) {
        return {};
    }
    const auto& [cardId_It, cardId_audioFragment] = *cardIdIt_group;
    if (cardId_It != std::prev(cardId_audioFragment.end())) {
        return {cardId_audioFragment.rbegin()->first};
    }
    return {};
}

auto CardAudioGroupDB::skipBackward(CardId cardId) const -> std::optional<CardId>
{
    const auto cardIdIt_group = cardIdIt_and_group(cardId);
    if (!cardIdIt_group.has_value()) {
        return {};
    }
    const auto& [cardId_It, cardId_audioFragment] = *cardIdIt_group;
    if (cardId_It != cardId_audioFragment.begin()) {
        return {cardId_audioFragment.begin()->first};
    }
    return {};
}

auto CardAudioGroupDB::findAudioGroupFromCardId(CardId cardId) const -> std::optional<uint>
{
    auto groupIt = ranges::find_if(
            id_cardAudioGroup,
            [cardId](const CardAudioGroup& cardAudioGroup) {
                return cardAudioGroup.cardId_audioFragment.contains(cardId);
            },
            &decltype(id_cardAudioGroup)::value_type::second);
    if (groupIt != id_cardAudioGroup.end()) {
        return {groupIt->first};
    }
    return {};
}

auto CardAudioGroupDB::cardIdIt_and_group(CardId cardId) const -> std::optional<
                                                                       std::pair<decltype(std::ranges::begin(id_cardAudioGroup.begin()->second.cardId_audioFragment)),
                                                                                 const decltype(CardAudioGroup().cardId_audioFragment)&>>
{
    auto audioGroupId = findAudioGroupFromCardId(cardId);
    if (not audioGroupId.has_value()) {
        return {};
    }
    const auto& cardId_audioFragment = id_cardAudioGroup.at(*audioGroupId).cardId_audioFragment;
    const auto cardId_It = ranges::find(
            cardId_audioFragment,
            cardId,
            &std::remove_reference_t<decltype(cardId_audioFragment)>::value_type::first);
    return {{cardId_It, cardId_audioFragment}};
}
