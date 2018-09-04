
#pragma once

#include "Math/Vector.h"

#include <string>
#include <vector>

struct SnapPoint
{
    unsigned int id = 0;
    float normal = 0;
    math::Vector position;
};

struct PrefabDefinition
{
    std::string name;
    std::string sprite_file;
    math::Vector scale;
    std::vector<math::Vector> collision_shape;
    std::vector<SnapPoint> snap_points;
};

std::vector<PrefabDefinition> LoadPrefabDefinitions();
const PrefabDefinition* FindPrefabFromName(const std::string& prefab_name, const std::vector<PrefabDefinition>& prefabs);
