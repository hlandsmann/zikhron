#pragma once
#include "IdGenerator.h"
#include "Video.h"
#include "WordDB.h"

#include <annotation/Tokenizer.h>
#include <misc/Identifier.h>

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace database {

class VideoSet
{
    static constexpr std::string_view s_type = "videoSet";

public:
    VideoSet(std::filesystem::path videoSetFile,
             std::shared_ptr<PackIdGenerator> packIdGenerator,
             std::shared_ptr<CardIdGenerator> cardIdGenerator,
             std::shared_ptr<annotation::Tokenizer> tokenizer,
             std::shared_ptr<WordDB> wordDB);
    VideoSet(std::filesystem::path videoSetFile,
             std::string name,
             const std::vector<std::filesystem::path>& videoFiles,
             std::shared_ptr<PackIdGenerator> packIdGenerator,
             std::shared_ptr<CardIdGenerator> cardIdGenerator,
             std::shared_ptr<annotation::Tokenizer> tokenizer,
             std::shared_ptr<WordDB> wordDB);
    [[nodiscard]] auto getName() const -> const std::string&;
    [[nodiscard]] auto getVideos() const -> const std::map<PackId, VideoPtr>&;
    void save();
    void saveProgress();

private:
    void deserialize();
    [[nodiscard]] auto serialize() const -> std::string;
    static auto genVideosFromPaths(const std::vector<std::filesystem::path>& videoFiles,
                                   const std::filesystem::path& videoSetFile,
                                   std::shared_ptr<PackIdGenerator> packIdGenerator,
                                   std::shared_ptr<CardIdGenerator> cardIdGenerator,
                                   std::shared_ptr<annotation::Tokenizer> tokenizer,
                                   std::shared_ptr<WordDB> wordDB) -> std::map<PackId, VideoPtr>;
    std::filesystem::path videoSetFile;
    std::string name;
    std::shared_ptr<PackIdGenerator> packIdGenerator;
    std::shared_ptr<CardIdGenerator> cardIdGenerator;
    std::shared_ptr<annotation::Tokenizer> tokenizer;
    std::shared_ptr<WordDB> wordDB;

    std::map<PackId, VideoPtr> videos;
};

using VideoSetPtr = std::shared_ptr<VideoSet>;

} // namespace database
