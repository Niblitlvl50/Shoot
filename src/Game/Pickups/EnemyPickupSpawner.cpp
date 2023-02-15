
#include "EnemyPickupSpawner.h"
#include "EnemyPickupLogic.h"
#include "Entity/EntityLogicSystem.h"
#include "Entity/Component.h"
#include "EntitySystem/IEntityManager.h"
#include "Player/PlayerInfo.h"
#include "Util/Random.h"
#include "TransformSystem/TransformSystem.h"

#include "System/File.h"
#include "nlohmann/json.hpp"

using namespace game;

EnemyPickupSpawner::EnemyPickupSpawner(
    DamageSystem* damage_system,
    EntityLogicSystem* logic_system,
    mono::TransformSystem* transform_system,
    mono::IEntityManager* entity_manager)
    : m_damage_system(damage_system)
    , m_logic_system(logic_system)
    , m_transform_system(transform_system)
    , m_entity_manager(entity_manager)
{
    const DamageCallback handle_destroyed_entity = [this](uint32_t id, int damage, uint32_t who_did_damage, DamageType type) {
        HandleSpawnEnemyPickup(id, damage, who_did_damage, type);
    };
    m_damage_callback_id = m_damage_system->SetGlobalDamageCallback(DamageType::DESTROYED, handle_destroyed_entity);

    file::FilePtr config_file = file::OpenAsciiFile("res/configs/pickup_config.json");
    if(config_file)
    {
        const std::vector<byte>& file_data = file::FileRead(config_file);
        const nlohmann::json& json = nlohmann::json::parse(file_data);

        for(const auto& pickup_config : json["pickups"])
        {
            PickupDefinition pickup_def;
            pickup_def.entity_file = pickup_config["entity_file"];
            pickup_def.drop_chance_percentage = pickup_config["drop_chance"];
            m_pickup_definitions.push_back(pickup_def);
        }
    }
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

    const bool initial_spawn_pickup = mono::Chance(50);
    const bool is_boss = m_damage_system->IsBoss(id);

    if(!initial_spawn_pickup && !is_boss)
        return;

    const int n_pickups = mono::RandomInt(2, 4);

    for(int index = 0; index < n_pickups; ++index)
    {
        const int picked_index = mono::RandomInt(0, m_pickup_definitions.size() - 1);
        const PickupDefinition& pickup_definition = m_pickup_definitions[picked_index];

        const bool spawn_pickup = mono::Chance(pickup_definition.drop_chance_percentage);
        if(!spawn_pickup)
            continue;

        mono::Entity spawned_entity = m_entity_manager->CreateEntity(pickup_definition.entity_file.c_str());
        m_entity_manager->AddComponent(spawned_entity.id, BEHAVIOUR_COMPONENT);
        m_logic_system->AddLogic(spawned_entity.id, new EnemyPickupLogic(spawned_entity.id, m_entity_manager));

        const float zero_to_tau = mono::Random(0.0f, math::TAU());
        const math::Vector random_offset = math::VectorFromAngle(zero_to_tau) * 0.5f;

        math::Matrix transform = m_transform_system->GetWorld(id);
        math::Translate(transform, random_offset);

        m_transform_system->SetTransform(spawned_entity.id, transform);
        m_transform_system->SetTransformState(spawned_entity.id, mono::TransformState::CLIENT);
    }
}
