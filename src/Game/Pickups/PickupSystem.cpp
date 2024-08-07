
#include "PickupSystem.h"
#include "DamageSystem/DamageSystem.h"
#include "SpawnSystem/SpawnSystem.h"

#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "EntitySystem/IEntityManager.h"
#include "EntitySystem/Entity.h"
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
    game::SpawnSystem* spawn_system,
    mono::TransformSystem* transform_system,
    mono::ParticleSystem* particle_system,
    mono::PhysicsSystem* physics_system,
    mono::IEntityManager* entity_manager)
    : m_damage_system(damage_system)
    , m_spawn_system(spawn_system)
    , m_transform_system(transform_system)
    , m_particle_system(particle_system)
    , m_physics_system(physics_system)
    , m_entity_manager(entity_manager)
    , m_pickups(n)
    , m_lootboxes(n)
{
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

        for(const auto& lootbox_pickup_config : json["lootbox_pickup"])
        {
            PickupDefinition pickup_def;
            pickup_def.entity_file = lootbox_pickup_config["entity_file"];
            pickup_def.drop_chance_percentage = lootbox_pickup_config["drop_chance"];
            m_lootbox_pickup_definitions.push_back(pickup_def);
        }

        for(const auto& horde_pickup_config : json["horde_pickups"])
        {
            PickupDefinition pickup_def;
            pickup_def.entity_file = horde_pickup_config["entity_file"];
            pickup_def.drop_chance_percentage = horde_pickup_config["drop_chance"];
            m_lootbox_definition.push_back(pickup_def);
        }

        m_pickup_annotation_entity = json["pickup_annotation"];
    }

    m_pickup_sound = audio::CreateSound("res/sound/pickups/money-pickup.wav", audio::SoundPlayback::ONCE);
    m_coins_sound = audio::CreateSound("res/sound/pickups/pickup_gold.wav", audio::SoundPlayback::ONCE);
    m_experience_sound = audio::CreateSound("res/sound/pickups/pickup_gold.wav", audio::SoundPlayback::ONCE);
    m_lootbox_sound = audio::CreateSound("res/sound/pickups/smoke_bomb.wav", audio::SoundPlayback::ONCE);
}

void PickupSystem::Begin()
{
    m_pickup_effect = new game::PickupEffect(m_particle_system, m_entity_manager);
    m_pickup_loot_effect = new game::PickupLootEffect(m_particle_system, m_entity_manager);

    const game::DamageCallback handle_destroyed_entity = [this](uint32_t damaged_entity_id, uint32_t who_did_damage, uint32_t weapon_identifier, int damage, DamageType type) {
        HandleSpawnEnemyPickup(damaged_entity_id);
    };
    m_damage_callback_id = m_damage_system->SetGlobalDamageCallback(DamageType::DESTROYED, handle_destroyed_entity);

    const game::SpawnSystem::SpawnCallback annotate_spawned_entity = [this](uint32_t entity_id, int spawn_score) {
        HandleEnemySpawn(entity_id, spawn_score);
    };
    m_spawn_callback_id = m_spawn_system->AddGlobalSpawnCallback(annotate_spawned_entity);
}

void PickupSystem::Reset()
{
    delete m_pickup_effect;
    m_pickup_effect = nullptr;

    delete m_pickup_loot_effect;
    m_pickup_loot_effect = nullptr;

    const auto release_callback = [this](uint32_t index, game::LootBox& loot_box) {
        m_entity_manager->RemoveReleaseCallback(index, loot_box.release_handle);
        loot_box.release_handle = mono::INVALID_ID;
    };
    m_lootboxes.ForEach(release_callback);

    m_damage_system->RemoveGlobalDamageCallback(m_damage_callback_id);
    m_spawn_system->RemoveGlobalSpawnCallback(m_spawn_callback_id);
}

game::Pickup* PickupSystem::AllocatePickup(uint32_t entity_id)
{
    game::Pickup* allocated_pickup = m_pickups.Set(entity_id, game::Pickup());

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

    m_pickups.Release(entity_id);
    m_collision_handlers[entity_id] = nullptr;
}

void PickupSystem::SetPickupData(uint32_t id, const Pickup& pickup_data)
{
    Pickup* pickup = m_pickups.Get(id);
    (*pickup) = pickup_data;
}

LootBox* PickupSystem::AllocateLootBox(uint32_t id)
{
    LootBox* loot_box = m_lootboxes.Set(id, game::LootBox());

    const mono::ReleaseCallback& on_release = [this](uint32_t entity_id, mono::ReleasePhase phase) {
        HandleReleaseLootBox(entity_id);
    };
    loot_box->release_handle = m_entity_manager->AddReleaseCallback(id, mono::ReleasePhase::PRE_RELEASE, on_release);

    return loot_box;
}

void PickupSystem::ReleaseLootBox(uint32_t id)
{
    LootBox* loot_box = m_lootboxes.Get(id);

    if(loot_box->release_handle != mono::INVALID_ID)
    {
        m_entity_manager->RemoveReleaseCallback(id, loot_box->release_handle);
        loot_box->release_handle = mono::INVALID_ID;
    }

    m_lootboxes.Release(id);
}

void PickupSystem::SetLootBoxData(uint32_t id, float temp)
{
    LootBox* loot_box = m_lootboxes.Get(id);
    loot_box->value = temp;
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

uint32_t PickupSystem::SpawnLootBox(const math::Vector& world_position) const
{
    const int picked_index = mono::RandomInt(0, m_lootbox_definition.size() - 1);
    const PickupDefinition& pickup_definition = m_lootbox_definition[picked_index];
    const mono::Entity spawned_entity = m_entity_manager->SpawnEntity(pickup_definition.entity_file.c_str());
    m_transform_system->SetTransform(spawned_entity.id, math::CreateMatrixWithPosition(world_position), mono::TransformState::CLIENT);

    return spawned_entity.id;
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
            const Pickup* pickup_data = m_pickups.Get(pickup.pickup_id);
            it->second(pickup_data->type, pickup_data->meta_data);
            PlayPickupSound(pickup_data->type);
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

void PickupSystem::HandleReleaseLootBox(uint32_t id)
{
    const int n_pickups = mono::RandomInt(5, 10);
    const math::Matrix& transform = m_transform_system->GetWorld(id);

    for(int index = 0; index < n_pickups; ++index)
    {
        const int picked_index = mono::RandomInt(0, m_lootbox_pickup_definitions.size() - 1);
        const PickupDefinition& pickup_definition = m_lootbox_pickup_definitions[picked_index];

        const bool spawn_pickup = mono::Chance(pickup_definition.drop_chance_percentage);
        if(!spawn_pickup)
            continue;

        const float zero_to_tau = mono::Random(0.0f, math::TAU());
        const math::Vector random_offset = math::VectorFromAngle(zero_to_tau) * mono::Random(0.25f, 1.0f);

        math::Matrix pickup_transform = transform;
        math::Translate(pickup_transform, random_offset);

        mono::Entity spawned_entity = m_entity_manager->SpawnEntity(pickup_definition.entity_file.c_str());
        m_transform_system->SetTransform(spawned_entity.id, pickup_transform);
        m_transform_system->SetTransformState(spawned_entity.id, mono::TransformState::CLIENT);

        m_spawned_pickups.push_back({ spawned_entity.id, 5.0f + mono::Random() });
    }

    m_pickup_loot_effect->EmitAt(math::GetPosition(transform));
    m_lootbox_sound->Play();
}

void PickupSystem::HandleSpawnEnemyPickup(uint32_t id)
{
    const bool is_player = game::IsPlayer(id);
    if(is_player)
        return;

    const bool garanteed_drop = (m_garanteed_drop.count(id) != 0);
    const bool initial_spawn_pickup = garanteed_drop || mono::Chance(25);

    m_garanteed_drop.erase(id);

    if(!initial_spawn_pickup)
        return;

    const int n_pickups = 1; //mono::RandomInt(2, 4);

    for(int index = 0; index < n_pickups; ++index)
    {
        const int picked_index = mono::RandomInt(0, m_pickup_definitions.size() - 1);
        const PickupDefinition& pickup_definition = m_pickup_definitions[picked_index];

        const bool spawn_pickup = garanteed_drop || mono::Chance(pickup_definition.drop_chance_percentage);
        if(!spawn_pickup)
            continue;

        const float zero_to_tau = mono::Random(0.0f, math::TAU());
        const math::Vector random_offset = math::VectorFromAngle(zero_to_tau) * 0.5f;

        math::Matrix transform = m_transform_system->GetWorld(id);
        math::Translate(transform, random_offset);

        mono::Entity spawned_entity = m_entity_manager->SpawnEntity(pickup_definition.entity_file.c_str());
        m_transform_system->SetTransform(spawned_entity.id, transform, mono::TransformState::CLIENT);

        m_spawned_pickups.push_back({ spawned_entity.id, 5.0f + mono::Random() });
    }
}

void PickupSystem::HandleEnemySpawn(uint32_t entity_id, int spawn_score)
{
    const bool is_player = game::IsPlayer(entity_id);
    if(is_player)
        return;

    const bool initial_spawn_pickup = mono::Chance(25);
    const bool is_boss = m_damage_system->IsBoss(entity_id);

    if(!initial_spawn_pickup && !is_boss)
        return;

    m_garanteed_drop.insert(entity_id);

    const math::Quad& bb = m_transform_system->GetBoundingBox(entity_id);
    const math::Vector& top_right = math::TopRight(bb) / 2.0f;
    const math::Matrix transform = math::CreateMatrixWithPosition(top_right);

    mono::Entity spawned_entity = m_entity_manager->SpawnEntity(m_pickup_annotation_entity.c_str());
    m_transform_system->SetTransform(spawned_entity.id, transform, mono::TransformState::CLIENT);
    m_transform_system->ChildTransform(spawned_entity.id, entity_id);
    m_entity_manager->SetLifetimeDependency(entity_id, spawned_entity.id);
}

void PickupSystem::PlayPickupSound(PickupType type)
{
    switch(type)
    {
    case PickupType::COINS:
        m_coins_sound->Play();
        break;
    case PickupType::EXPERIENCE:
        m_experience_sound->Play();
        break;
    default:
        m_pickup_sound->Play();
        break;
    };
}
