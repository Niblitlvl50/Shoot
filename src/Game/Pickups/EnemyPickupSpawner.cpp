
#include "EnemyPickupSpawner.h"
#include "EntitySystem/IEntityManager.h"
#include "Player/PlayerInfo.h"
#include "Util/Random.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

EnemyPickupSpawner::EnemyPickupSpawner(
    DamageSystem* damage_system, mono::TransformSystem* transform_system, mono::IEntityManager* entity_manager)
    : m_damage_system(damage_system)
    , m_transform_system(transform_system)
    , m_entity_manager(entity_manager)
{
    const DamageCallback handle_destroyed_entity = [this](uint32_t id, int damage, uint32_t who_did_damage, DamageType type) {
        HandleSpawnEnemyPickup(id, damage, who_did_damage, type);
    };
    m_damage_callback_id = m_damage_system->SetGlobalDamageCallback(DamageType::DESTROYED, handle_destroyed_entity);
}

EnemyPickupSpawner::~EnemyPickupSpawner()
{
    m_damage_system->RemoveGlobalDamageCallback(m_damage_callback_id);
}

void EnemyPickupSpawner::HandleSpawnEnemyPickup(uint32_t id, int damage, uint32_t who_did_damage, DamageType type)
{
    const bool is_player = game::IsPlayer(id);
    if(is_player)
        return;

    constexpr const char* known_pickups[] = {
        "res/entities/pickup_health_10.entity",
        "res/entities/pickup_ammo_10.entity"
    };

    const bool success = mono::Chance(10);
    if(success)
    {
        const int picked_index = mono::RandomInt(0, std::size(known_pickups) - 1);
        mono::Entity spawned_entity = m_entity_manager->CreateEntity(known_pickups[picked_index]);

        const math::Matrix& transform = m_transform_system->GetWorld(id);
        m_transform_system->SetTransform(spawned_entity.id, transform);
        m_transform_system->SetTransformState(spawned_entity.id, mono::TransformState::CLIENT);
    }
}
