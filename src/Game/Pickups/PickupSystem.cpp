
#include "PickupSystem.h"
#include "DamageSystem/DamageSystem.h"
#include "Entity/EntityLogicSystem.h"

#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "EntitySystem/IEntityManager.h"
#include "Player/PlayerInfo.h"

#include "System/File.h"
#include "System/Hash.h"
#include "Util/Random.h"
#include "Util/Algorithm.h"

#include "Effects/PickupEffect.h"
#include "Entity/Component.h"

#include "nlohmann/json.hpp"

using namespace game;

namespace
{
    class PickupHandler : public mono::ICollisionHandler
    {
    public:

        PickupHandler(PickupSystem* pickup_system, uint32_t pickup_id)
            : m_pickup_system(pickup_system)
            , m_pickup_id(pickup_id)
        { }

        mono::CollisionResolve OnCollideWith(
            mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t categories) override
        {
            const uint32_t target_id = mono::PhysicsSystem::GetIdFromBody(body);
            m_pickup_system->HandlePickup(m_pickup_id, target_id);

            return mono::CollisionResolve::IGNORE;
        }

        void OnSeparateFrom(mono::IBody* body) override
        { }

        PickupSystem* m_pickup_system;
        uint32_t m_pickup_id;
    };
}

PickupSystem::PickupSystem(
    uint32_t n,
    game::DamageSystem* damage_system,
    mono::TransformSystem* transform_system,
    mono::ParticleSystem* particle_system,
    mono::PhysicsSystem* physics_system,
    mono::IEntityManager* entity_manager)
    : m_damage_system(damage_system)
    , m_transform_system(transform_system)
    , m_particle_system(particle_system)
    , m_physics_system(physics_system)
    , m_entity_manager(entity_manager)
{
    m_pickups.resize(n);
    m_active.resize(n, false);
    m_collision_handlers.resize(n);

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

    m_pickup_sound = audio::CreateSound("res/sound/pickups/money-pickup.wav", audio::SoundPlayback::ONCE);
    m_coins_sound = audio::CreateSound("res/sound/pickups/pickup_gold.wav", audio::SoundPlayback::ONCE);
}

void PickupSystem::Begin()
{
    m_pickup_effect = new game::PickupEffect(m_particle_system, m_entity_manager);

    const game::DamageCallback handle_destroyed_entity = [this](uint32_t id, int damage, uint32_t who_did_damage, DamageType type) {
        HandleSpawnEnemyPickup(id, damage, who_did_damage, type);
    };
    m_damage_callback_id = m_damage_system->SetGlobalDamageCallback(DamageType::DESTROYED, handle_destroyed_entity);
}

void PickupSystem::Reset()
{
    delete m_pickup_effect;
    m_pickup_effect = nullptr;

    m_damage_system->RemoveGlobalDamageCallback(m_damage_callback_id);
}

game::Pickup* PickupSystem::AllocatePickup(uint32_t entity_id)
{
    m_active[entity_id] = true;
    game::Pickup* allocated_pickup = &m_pickups[entity_id];

    mono::IBody* body = m_physics_system->GetBody(entity_id);
    if(body)
    {
        auto handler = std::make_unique<PickupHandler>(this, entity_id);
        body->AddCollisionHandler(handler.get());
        m_collision_handlers[entity_id] = std::move(handler);
    }

    return allocated_pickup;
}

void PickupSystem::ReleasePickup(uint32_t entity_id)
{
    auto& handler = m_collision_handlers[entity_id];

    mono::IBody* body = m_physics_system->GetBody(entity_id);
    if(body)
        body->RemoveCollisionHandler(handler.get());

    m_active[entity_id] = false;
    m_collision_handlers[entity_id] = nullptr;
}

void PickupSystem::SetPickupData(uint32_t id, const Pickup& pickup_data)
{
    m_pickups[id] = pickup_data;
}

void PickupSystem::HandlePickup(uint32_t pickup_id, uint32_t target_id)
{
    m_pickups_to_process.push_back( { pickup_id, target_id } );

    const auto remove_by_id = [pickup_id](const SpawnedPickup& pickup) {
        return pickup.pickup_id == pickup_id;
    };
    mono::remove_if(m_spawned_pickups, remove_by_id);
}

void PickupSystem::RegisterPickupTarget(uint32_t target_id, PickupCallback callback)
{
    m_pickup_targets[target_id] = callback;
}

void PickupSystem::UnregisterPickupTarget(uint32_t target_id)
{
    m_pickup_targets.erase(target_id);
}

const char* PickupSystem::Name() const
{
    return "pickupsystem";
}

void PickupSystem::Update(const mono::UpdateContext& update_context)
{
    for(const PickupToTarget& pickup : m_pickups_to_process)
    {
        const auto it = m_pickup_targets.find(pickup.target_id);
        if(it != m_pickup_targets.end())
        {
            const Pickup& pickup_data = m_pickups[pickup.pickup_id];
            it->second(pickup_data.type, pickup_data.amount);
            PlayPickupSound(pickup_data.type);
        }

        const math::Vector& world_position = m_transform_system->GetWorldPosition(pickup.pickup_id);
        m_pickup_effect->EmitAt(world_position);
        m_entity_manager->ReleaseEntity(pickup.pickup_id);
    }
    m_pickups_to_process.clear();

    const auto decrement_lifetime_and_remove = [this, &update_context](SpawnedPickup& pickup) {

        pickup.lifetime -= update_context.delta_s;
        
        const bool remove_pickup = (pickup.lifetime < 0.0f);
        if(remove_pickup)
        {
            const math::Vector& world_position = m_transform_system->GetWorldPosition(pickup.pickup_id);
            m_pickup_effect->EmitAt(world_position);
            m_entity_manager->ReleaseEntity(pickup.pickup_id);
        }

        return remove_pickup;
    };
    mono::remove_if(m_spawned_pickups, decrement_lifetime_and_remove);
}

void PickupSystem::HandleSpawnEnemyPickup(uint32_t id, int damage, uint32_t who_did_damage, DamageType type)
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

        const float zero_to_tau = mono::Random(0.0f, math::TAU());
        const math::Vector random_offset = math::VectorFromAngle(zero_to_tau) * 0.5f;

        mono::Entity spawned_entity = m_entity_manager->SpawnEntity(pickup_definition.entity_file.c_str());

        math::Matrix transform = m_transform_system->GetWorld(id);
        math::Translate(transform, random_offset);

        m_transform_system->SetTransform(spawned_entity.id, transform);
        m_transform_system->SetTransformState(spawned_entity.id, mono::TransformState::CLIENT);

        m_spawned_pickups.push_back({ spawned_entity.id, 5.0f + mono::Random() });
    }
}

void PickupSystem::PlayPickupSound(PickupType type)
{
    switch(type)
    {
    case PickupType::COINS:
        m_coins_sound->Play();
        break;
    default:
        m_pickup_sound->Play();
        break;
    };
}
