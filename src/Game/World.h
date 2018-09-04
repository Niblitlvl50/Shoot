
#pragma once

#include "MonoFwd.h"
#include "WorldFile.h"

#include "Spawner.h"
#include "Enemies/IEnemyFactory.h"
#include "GameObjects/IGameObjectFactory.h"

namespace game
{    
    struct Ammo;
    
    void LoadWorld(
        mono::IPhysicsZone* zone, const std::vector<world::PolygonData>& polygons, const std::vector<world::PrefabData>& prefabs);
    
    void LoadWorldObjects(
        const std::vector<world::WorldObject>& objects,
        IEnemyFactory* enemy_factory,
        IGameObjectFactory* gameobject_factory,
        std::vector<game::EnemyPtr>& enemies,
        std::vector<mono::IPhysicsEntityPtr>& gameobjects,
        std::vector<SpawnPoint>& spawn_points,
        std::vector<math::Vector>& player_points,
        std::vector<game::Ammo>& ammo_pickups);

    void LoadPrefabs();
}
