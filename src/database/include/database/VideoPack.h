#pragma once
#include <filesystem>

namespace database {

class VideoPack
{
public:
    VideoPack(std::filesystem::path videoPackFile);

private:
    std::filesystem::path videoPackFile;
};

} // namespace annotation
