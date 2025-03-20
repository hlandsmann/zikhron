#pragma once
#include "Mecab.h"

#include <spdlog/common.h>
#include <spdlog/logger.h>

#include <memory>
#include <string>
#include <vector>

namespace MeCab {
class Tagger;
}

namespace annotation {

class MecabWrapper
{
public:
    MecabWrapper();
    [[nodiscard]] auto split(const std::string& text) const -> std::vector<MecabToken>;

    void setDebugSink(spdlog::sink_ptr sink);

private:
    std::shared_ptr<MeCab::Tagger> tagger;
    std::unique_ptr<spdlog::logger> log;
};
} // namespace annotation
