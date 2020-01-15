
#pragma once

#include "MonoFwd.h"
#include "WorldFile.h"

#include "Spawner.h"

namespace game
{    
    struct Pickup;
    struct ExcludeZone;
    
    void LoadWorld(
        //mono::IPhysicsZone* zone,
        const std::vector<world::PolygonData>& polygons,
        std::vector<ExcludeZone>& exclude_zones);
    
    void LoadWorldObjects(
        const std::vector<world::WorldObject>& objects,
        std::vector<SpawnPoint>& spawn_points,
        std::vector<math::Vector>& player_points,
        std::vector<game::Pickup>& ammo_pickups);
}
