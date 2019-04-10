
#pragma once

#include <cstdint>

namespace game
{
    class IEntityLogic
    {
    public:

        virtual ~IEntityLogic() = default;
        virtual void Update(uint32_t delta_ms) = 0;
    };
}
