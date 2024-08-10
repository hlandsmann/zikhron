#pragma once
#include "IdGenerator.h"
#include "VideoSet.h"

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
            std::shared_ptr<CardIdGenerator> cardIdGenerator);
    auto addVideoSet(const std::vector<std::filesystem::path>& videoFiles) -> VideoSetPtr;
    [[nodiscard]] auto getVideoSets() const -> const std::vector<VideoSetPtr>&;
    void save();

private:
    [[nodiscard]] static auto loadVideoSets(const std::filesystem::path& directory,
                                            const std::shared_ptr<PackIdGenerator>& packIdGenerator,
                                            const std::shared_ptr<CardIdGenerator>& cardIdGenerator)
            -> std::vector<VideoSetPtr>;

    std::shared_ptr<PackIdGenerator> packIdGenerator;
    std::shared_ptr<CardIdGenerator> cardIdGenerator;
    std::filesystem::path videoSetDir;
    std::vector<VideoSetPtr> videoSets;
};

} // namespace database
