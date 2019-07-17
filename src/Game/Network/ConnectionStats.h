
#pragma once

#include <cstdint>

namespace game
{
    struct ConnectionStats
    {
        uint32_t total_packages_sent;
        uint32_t total_packages_received;

        uint32_t total_byte_sent;
        uint32_t total_byte_received;

        uint32_t total_compressed_byte_sent;
        uint32_t total_compressed_byte_received;
    };
}
