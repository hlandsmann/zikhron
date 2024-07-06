#include "VideoPack.h"

#include <filesystem>
#include <utility>

namespace database {

VideoPack::VideoPack(std::filesystem::path _videoPackFile)
    : videoPackFile{std::move(_videoPackFile)}

{}

} // namespace database
