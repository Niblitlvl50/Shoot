
#include "SpawnPredictionSystem.h"
#include "Network/ClientManager.h"
#include "DamageSystem.h"

#include "EntitySystem/IEntityManager.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Util/Hash.h"
#include "Util/Algorithm.h"

using namespace game;

SpawnPredictionSystem::SpawnPredictionSystem(
    const ClientManager* client_manager,
    mono::SpriteSystem* sprite_system,
    game::DamageSystem* damage_system,
    mono::IEntityManager* entity_manager)
    : m_client_manager(client_manager)
    , m_sprite_system(sprite_system)
    , m_damage_system(damage_system)
    , m_entity_manager(entity_manager)
{
    m_spawn_messages.reserve(50);
}

void SpawnPredictionSystem::HandleSpawnMessage(const SpawnMessage& spawn_message)
{
    // Only despawns for now!
    if(!spawn_message.spawn)
        m_spawn_messages.push_back(spawn_message);
}

uint32_t SpawnPredictionSystem::Id() const
{
    return mono::Hash(Name());
}

const char* SpawnPredictionSystem::Name() const
{
    return "spawnpredictionsystem";
}

void SpawnPredictionSystem::Update(const mono::UpdateContext& update_context)
{
    const uint32_t server_time = m_client_manager->GetServerTimePredicted();
    if(server_time == 0)
        return;

    std::vector<uint32_t> entities_to_despawn;

    const auto update_and_remove = [&](SpawnMessage& message) {
        const bool despawn_entity = (message.timestamp <= server_time);
        if(despawn_entity)
            entities_to_despawn.push_back(message.entity_id);

        return despawn_entity;
    };

    mono::remove_if(m_spawn_messages, update_and_remove);

    for(uint32_t entity_id : entities_to_despawn)
    {
        const bool is_sprite_allocated = m_sprite_system->IsAllocated(entity_id);
        if(is_sprite_allocated)
            m_sprite_system->ReleaseSprite(entity_id);
    
        const bool is_damageinfo_allocated = m_damage_system->IsAllocated(entity_id);
        if(is_damageinfo_allocated)
            m_damage_system->ReleaseRecord(entity_id);
    
        //m_entity_manager->ReleaseEntity(entity_id);
    }
}
