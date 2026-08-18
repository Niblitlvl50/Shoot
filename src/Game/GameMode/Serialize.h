
#pragma once

#include "HordeGameMode.h"
#include "nlohmann/json.hpp"

namespace game
{
    inline void from_json(const nlohmann::json& json, HordeConfig& config)
    {
        config.num_waves             = json.value("num_waves",             config.num_waves);
        config.initial_wave_delay_s  = json.value("initial_wave_delay_s",  config.initial_wave_delay_s);
        config.wave_interval_s       = json.value("wave_interval_s",       config.wave_interval_s);
        config.spawn_score_increment = json.value("spawn_score_increment", config.spawn_score_increment);
    }
}
