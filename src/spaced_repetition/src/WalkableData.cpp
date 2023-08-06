#include <WalkableData.h>

WalkableData::WalkableData(std::shared_ptr<zikhron::Config> config)
    : db{std::move(config)}
{

}
