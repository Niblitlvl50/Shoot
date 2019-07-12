
#pragma once

#include <cstdint>

namespace game
{
    struct ConnectionStats
    {
        uint32_t total_sent;
        uint32_t total_received;
        uint32_t total_byte_sent;
        uint32_t total_byte_received;
    };
}
