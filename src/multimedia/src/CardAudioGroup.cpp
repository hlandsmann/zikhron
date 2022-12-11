#include <CardAudioGroup.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <ctre.hpp>
#include <fstream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <ranges>

namespace ranges = std::ranges;
namespace fs = std::filesystem;
using json = nlohmann::json;

auto CardAudioGroup::toJson(const CardAudioGroup& self) -> json {
    json cardAudioGroup = json::object();
    cardAudioGroup[std::string(s_audioFile)] = self.audioFile;

    json json_cardId_fragment = json::object();
    for (const auto& [cardId, audioFragment] : self.cardId_audioFragment) {
        json jsonFragment = json::object();
        jsonFragment[std::string(s_start)] = audioFragment.start;
        jsonFragment[std::string(s_end)] = audioFragment.end;
        json_cardId_fragment[std::to_string(cardId)] = jsonFragment;
    }

    cardAudioGroup[std::string(s_fragments)] = json_cardId_fragment;
    return cardAudioGroup;
}

auto CardAudioGroup::fromJson(const json& cag) -> CardAudioGroup {
    CardAudioGroup cardAudioGroup;
    auto& cardId_audioFragment = cardAudioGroup.cardId_audioFragment;
    cardAudioGroup.audioFile = std::string(cag[std::string(s_audioFile)]);
    ranges::transform(cag[std::string(s_fragments)].items(),
                      std::inserter(cardId_audioFragment, cardId_audioFragment.begin()),
                      [](const auto& cardId_fragment) -> std::pair<uint, AudioFragment> {
                          const auto& [cardId, fragment_json] = cardId_fragment;
                          AudioFragment fragment = {.start = fragment_json[s_start],
                                                    .end = fragment_json[s_end]};
                          return {std::stoi(cardId), fragment};
                      });
    return cardAudioGroup;
}

std::unique_ptr<CardAudioGroupDB> cardAudioGroupDB;
auto CardAudioGroupDB::get() -> CardAudioGroupDB& {
    if (!cardAudioGroupDB)
        cardAudioGroupDB = std::unique_ptr<CardAudioGroupDB>(new CardAudioGroupDB());
    return *cardAudioGroupDB;
}

CardAudioGroupDB::CardAudioGroupDB() { load(); }

void CardAudioGroupDB::load() {
    fs::path path_dir_cardAudioGroup = s_groupDir;
    if (!fs::exists(path_dir_cardAudioGroup))
        return;

    auto fn_matcher = ctre::match<"(\\d{6})(\\.group)">;
    for (const fs::path& entry : fs::directory_iterator(path_dir_cardAudioGroup)) {
        std::string fn = entry.filename().string();
        auto fn_match = fn_matcher(fn);
        if (not fn_match) {
            spdlog::warn(
                "File \"{}\" in \"{}\" ignored. Please remove!", fn, path_dir_cardAudioGroup.string());
            continue;
        }
        try {
            int groupId = std::stoi(fn_match.get<1>().to_string());
            std::ifstream groupFile(entry, std::ios::in | std::ios::binary);
            if (!groupFile)
                throw std::runtime_error("Failure to read file");
            if (id_cardAudioGroup.contains(groupId)) {
                spdlog::warn(
                    "GroupId {} ignored, because its already in use! Fn: {}", groupId, entry.string());
                continue;
            }
            id_cardAudioGroup[groupId] = CardAudioGroup::fromJson(json::parse(groupFile));
        }
        catch (std::exception& e) {
            spdlog::error("{} - file: {}", e.what(), entry.filename().string());
        }
    }
}

void CardAudioGroupDB::save(uint groupId, const CardAudioGroup& cardAudioGroup) {
    id_cardAudioGroup[groupId] = cardAudioGroup;

    fs::path path_dir_cardAudioGroup = s_groupDir;
    try {
        if (!fs::exists(path_dir_cardAudioGroup))
            fs::create_directory(path_dir_cardAudioGroup);

        fs::path fn_cardAudioGroup = path_dir_cardAudioGroup / fmt::format("{:06d}.group", groupId);
        std::ofstream ofs(fn_cardAudioGroup);
        ofs << CardAudioGroup::toJson(cardAudioGroup).dump(4);
        spdlog::debug("Saved {}", fn_cardAudioGroup.string());
    }
    catch (const std::exception& e) {
        spdlog::error("Failed to save file, exception: {}", e.what());
    }
}

auto CardAudioGroupDB::get_cardAudioGroup(uint groupId) const -> CardAudioGroup {
    if (id_cardAudioGroup.contains(groupId))
        return id_cardAudioGroup.at(groupId);
    else
        return {};
}