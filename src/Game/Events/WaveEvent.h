
#pragma once

namespace game
{
    struct WaveStartedEvent
    {
        WaveStartedEvent(const char* wave_name, int wave_index)
            : wave_name(wave_name)
            , wave_index(wave_index)  
        { }

        const char* wave_name;
        const int wave_index;
    };

    struct HordeCompletedEvent
    { };
}
