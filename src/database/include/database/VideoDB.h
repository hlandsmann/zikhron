#pragma once
#include "IdGenerator.h"
#include "VideoSet.h"
#include "WordDB.h"

#include <annotation/Tokenizer.h>
#include <misc/Config.h>

#include <filesystem>
#include <memory>
#include <vector>

namespace database {

class VideoDB
{
    static constexpr auto s_videoSubdirectory = "video";
    static constexpr auto s_videoSetExtension = ".vpkg";

public:
    VideoDB(std::shared_ptr<zikhron::Config> config,
            std::shared_ptr<PackIdGenerator> packIdGenerator,
            std::shared_ptr<CardIdGenerator> cardIdGenerator,
            std::shared_ptr<annotation::Tokenizer> tokenizer,
            std::shared_ptr<WordDB> wordDB);
    auto addVideoSet(const std::vector<std::filesystem::path>& videoFiles) -> VideoSetPtr;
    [[nodiscard]] auto getVideoSets() const -> const std::vector<VideoSetPtr>&;
    void save();
    void saveProgress();

private:
    [[nodiscard]] static auto loadVideoSets(const std::filesystem::path& directory,
                                            const std::shared_ptr<PackIdGenerator>& packIdGenerator,
                                            const std::shared_ptr<CardIdGenerator>& cardIdGenerator,
                                            std::shared_ptr<annotation::Tokenizer> tokenizer,
                                            std::shared_ptr<WordDB> wordDB)
            -> std::vector<VideoSetPtr>;

    std::shared_ptr<PackIdGenerator> packIdGenerator;
    std::shared_ptr<CardIdGenerator> cardIdGenerator;
    std::shared_ptr<annotation::Tokenizer> tokenizer;
    std::shared_ptr<WordDB> wordDB;
    std::filesystem::path videoSetDir;
    std::vector<VideoSetPtr> videoSets;
};

} // namespace database
