
#pragma once

namespace game
{
    struct PickupEvent
    {
        PickupEvent(unsigned int entity_id, int type, int value)
            : entity_id(entity_id)
            , type(type)
            , value(value)
        { }

        const unsigned int entity_id;
        const int type;
        const int value;
    };
}
