#pragma once
#include "IdGenerator.h"
#include "Subtitle.h"
#include "SubtitlePicker.h"
#include "WordDB.h"

#include <annotation/Tokenizer.h>
#include <misc/Identifier.h>
#include <multimedia/Subtitle.h>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace database {

class Video
{
public:
    Video(std::string_view sv,
          std::filesystem::path videoSetFile,
          PackId videoId,
          std::shared_ptr<CardIdGenerator> cardIdGenerator,
          std::shared_ptr<annotation::Tokenizer> tokenizer,
          std::shared_ptr<WordDB> wordDB);
    Video(std::filesystem::path videoFile,
          std::filesystem::path videoSetFile,
          PackId videoId,
          std::shared_ptr<CardIdGenerator> cardIdGenerator,
          std::shared_ptr<annotation::Tokenizer> tokenizer,
          std::shared_ptr<WordDB> wordDB);
    [[nodiscard]] auto serialize() const -> std::string;
    void loadSubtitles();
    [[nodiscard]] auto getVideoFile() const -> const std::filesystem::path&;
    [[nodiscard]] auto getActiveSubtitle() -> SubtitlePickerPtr;

private:
    void deserialize(std::string_view content);
    std::filesystem::path videoSetFile;
    std::filesystem::path videoFile;
    std::string name;
    PackId videoId;
    std::shared_ptr<CardIdGenerator> cardIdGenerator;
    std::shared_ptr<annotation::Tokenizer> tokenizer;
    std::shared_ptr<WordDB> wordDB;

    std::shared_ptr<SubtitlePicker> subtitlePicker;

    std::vector<SubtitlePtr> subtitles;
    std::size_t subChoice = 0;
};

using VideoPtr = std::shared_ptr<Video>;
} // namespace database
