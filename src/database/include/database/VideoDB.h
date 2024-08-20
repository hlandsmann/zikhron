#pragma once
#include "CbdFwd.h"
#include "IdGenerator.h"
#include "Video.h"
#include "VideoSet.h"
#include "WordDB.h"

#include <annotation/Tokenizer.h>
#include <misc/Config.h>
#include <misc/Identifier.h>

#include <filesystem>
#include <generator>
#include <map>
#include <memory>
#include <string>
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
    [[nodiscard]] auto getDeserializedCards() const -> std::generator<CardPtr>;
    auto addVideoSet(const std::vector<std::filesystem::path>& videoFiles) -> VideoSetPtr;
    [[nodiscard]] auto getVideoSets() const -> const std::vector<VideoSetPtr>&;
    [[nodiscard]] auto getVideos() const -> const std::map<PackId, VideoPtr>&;
    [[nodiscard]] auto getVideo(const std::string& videoName) const -> VideoPtr;
    void save();
    void saveProgress();

private:
    [[nodiscard]] static auto loadVideoSets(const std::filesystem::path& directory,
                                            const std::shared_ptr<PackIdGenerator>& packIdGenerator,
                                            const std::shared_ptr<CardIdGenerator>& cardIdGenerator,
                                            std::shared_ptr<annotation::Tokenizer> tokenizer,
                                            std::shared_ptr<WordDB> wordDB)
            -> std::vector<VideoSetPtr>;
    void addVideosFromVideoSet(const VideoSetPtr& videoSet);

    std::shared_ptr<PackIdGenerator> packIdGenerator;
    std::shared_ptr<CardIdGenerator> cardIdGenerator;
    std::shared_ptr<annotation::Tokenizer> tokenizer;
    std::shared_ptr<WordDB> wordDB;
    std::filesystem::path videoSetDir;
    std::vector<VideoSetPtr> videoSets;
    std::map<PackId, VideoPtr> videos;
    std::map<std::string /* name */, VideoPtr> nameToVideos;
};

} // namespace database
