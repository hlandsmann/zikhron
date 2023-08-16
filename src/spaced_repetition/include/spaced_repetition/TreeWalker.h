#pragma once

#include "WalkableData.h"

#include <memory>

namespace sr {
class TreeWalker
{
public:
    TreeWalker(std::shared_ptr<WalkableData>);

private:
    std::shared_ptr<WalkableData> walkableData;
};
} // namespace sr
