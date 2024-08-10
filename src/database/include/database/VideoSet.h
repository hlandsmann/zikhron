#pragma once
#include "IdGenerator.h"
#include "Video.h"

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
             std::shared_ptr<CardIdGenerator> cardIdGenerator);
    VideoSet(std::filesystem::path videoSetFile,
             std::string name,
             const std::vector<std::filesystem::path>& videoFiles,
             std::shared_ptr<PackIdGenerator> packIdGenerator,
             std::shared_ptr<CardIdGenerator> cardIdGenerator);
    [[nodiscard]] auto getName() const -> const std::string&;
    [[nodiscard]] auto getVideo() const -> VideoPtr;
    void save();

private:
    void deserialize();
    [[nodiscard]] auto serialize() const -> std::string;
    static auto genVideosFromPaths(const std::vector<std::filesystem::path>& videoFiles,
                                   const std::filesystem::path& videoSetFile,
                                   std::shared_ptr<PackIdGenerator> packIdGenerator,
                                   std::shared_ptr<CardIdGenerator> cardIdGenerator) -> std::map<PackId, VideoPtr>;
    std::filesystem::path videoSetFile;
    std::string name;
    std::shared_ptr<PackIdGenerator> packIdGenerator;
    std::shared_ptr<CardIdGenerator> cardIdGenerator;

    std::map<PackId, VideoPtr> videos;
};

using VideoSetPtr = std::shared_ptr<VideoSet>;

} // namespace database
