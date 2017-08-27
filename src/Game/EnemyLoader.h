
#pragma once

#include "Enemies/IEnemyFactory.h"
#include <vector>

namespace game
{
    std::vector<EnemyPtr> LoadEnemies(const char* object_file, IEnemyFactory* factory);
}
