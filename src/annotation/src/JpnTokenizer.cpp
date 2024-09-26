#include "JpnTokenizer.h"
// #include <spdlog/spdlog.h>

#include "Jumanpp.h"

namespace annotation {
JpnTokenizer::JpnTokenizer()
    : jumanppPtr{std::make_shared<jumanpp::Jumanpp>()}
{}

void JpnTokenizer::tokenize(const std::string& text)
{
    jumanppPtr->tokenize(text);
}
} // namespace japanese
