
#pragma once

#include "EntitySystem/IEntityManager.h"

namespace game
{
    std::vector<mono::EntityData> LoadEntityFile(const char* entity_file);
}
