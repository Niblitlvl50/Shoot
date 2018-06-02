
#pragma once

namespace game
{
    struct PickupEvent
    {
        PickupEvent(unsigned int entity_id, int value)
            : entity_id(entity_id)
            , value(value)
        { }

        const unsigned int entity_id;
        const int value;
    };
}
