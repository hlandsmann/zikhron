#include "Crc32.h"
#include <array>
#include <cstdint>
#include <string>

namespace {
auto crc32SingleValue(uint32_t value, uint32_t lastCRC) -> std::uint32_t
{
    constexpr std::array<uint32_t, 16> crcTable =
            {0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
             0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
             0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
             0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD};
    uint32_t crc = lastCRC;

    crc = crc ^ value;

    for (int i = 0; i < 8; i++) {
        crc = (crc << 4) ^ crcTable.at(crc >> 28);
    }
    return crc;
}
} // namespace

namespace utl {
auto calculateCrc32(const std::string& data) -> std::uint32_t
{
    uint32_t value = 0xFFFFFFFF;
    uint32_t crc = 0xFFFFFFFF;
    unsigned index = 0;
    for (; index < data.length(); index++) {
        auto byte = static_cast<uint8_t>(data[index]);
        value ^= uint32_t(~(byte | 0xFFFFFF00)) << ((index % 4) * 8);

        if (index % sizeof(uint32_t) == sizeof(uint32_t) - 1) {
            crc = crc32SingleValue(value, crc);
            value = 0xFFFFFFFF;
        }
    }
    if ((index % sizeof(uint32_t)) != 0) {
        crc = crc32SingleValue(value, crc);
    }

    return crc;
}
} // namespace utl
