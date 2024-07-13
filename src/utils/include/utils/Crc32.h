#pragma once
#include <cstdint>
#include <string>

namespace utl {

auto calculateCrc32(const std::string& data) -> std::uint32_t;

}
