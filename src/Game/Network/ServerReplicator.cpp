
#include "ServerReplicator.h"
#include "ServerManager.h"
#include "INetworkPipe.h"
#include "NetworkMessage.h"
#include "BatchedMessageSender.h"

#include "EventHandler/EventHandler.h"
#include "EntitySystem/EntitySystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "System/Hash.h"
#include "Util/Algorithm.h"

#include "EntitySystem/IEntityManager.h"
#include "Entity/EntityProperties.h"

#include "Events/GameEventFuncFwd.h"
#include "Events/PlayerEvents.h"
#include "Player/PlayerInfo.h"
#include "WorldFile.h"
#include "DamageSystem/DamageSystem.h"

#include "Math/MathFunctions.h"
#include "Camera/ICamera.h"
#include "Entity/Component.h"

#include <unordered_set>

using namespace game;

ServerReplicator::ServerReplicator(
    mono::EventHandler* event_handler,
    mono::EntitySystem* entity_system,
    mono::TransformSystem* transform_system,
    mono::SpriteSystem* sprite_system,
    DamageSystem* damage_system,
    ServerManager* server_manager,
    const LevelMetadata& level_metadata,
    uint32_t replication_interval)
    : m_event_handler(event_handler)
    , m_entity_system(entity_system)
    , m_transform_system(transform_system)
    , m_sprite_system(sprite_system)
    , m_damage_system(damage_system)
    , m_server_manager(server_manager)
    , m_replication_interval(replication_interval)
{
    std::memset(&m_transform_data, 0, sizeof(m_transform_data));
    std::memset(&m_sprite_data, 0, sizeof(m_sprite_data));
    std::memset(&m_health_data, 0, sizeof(m_health_data));

    const PlayerConnectedFunc connected_func = [server_manager, level_metadata](const PlayerConnectedEvent& event) {

        LevelMetadataMessage metadata_message;
        metadata_message.camera_position = level_metadata.camera_position;
        metadata_message.camera_size = level_metadata.camera_size;
        metadata_message.background_texture_hash = hash::Hash(level_metadata.background_texture.c_str());
        metadata_message.world_file_hash = 0;

        NetworkMessage message;
        message.address = event.address;
        message.payload = SerializeMessage(metadata_message);
        server_manager->SendMessage(message);

        return mono::EventResult::PASS_ON;
    };
    m_connected_token = m_event_handler->AddListener(connected_func);
}

ServerReplicator::~ServerReplicator()
{
    m_event_handler->RemoveListener(m_connected_token);
}

void ServerReplicator::Update(const mono::UpdateContext& update_context)
{
    //SCOPED_TIMER_AUTO();
    const std::unordered_map<network::Address, ClientData>& clients = m_server_manager->GetConnectedClients();
    if(!clients.empty())
    {
        std::vector<uint32_t> transforms_to_replicate;
        std::vector<uint32_t> sprites_to_replicate;
        std::vector<uint32_t> damage_info_to_replicate;
        std::vector<uint32_t> spawns_this_frame;

        const auto collect_entities = [&](const mono::Entity& entity) {
            transforms_to_replicate.push_back(entity.id);

            if(mono::contains(entity.components, SPRITE_COMPONENT))
                sprites_to_replicate.push_back(entity.id);

            if(mono::contains(entity.components, HEALTH_COMPONENT))
                damage_info_to_replicate.push_back(entity.id);
        };
        m_entity_system->ForEachEntity(collect_entities);

        for(const auto& spawn_event : m_entity_system->GetSpawnEvents())
        {
            if(spawn_event.spawned)
                spawns_this_frame.push_back(spawn_event.entity_id);
        }

        std::unordered_set<network::Address> known_clients;

        for(const auto& client : clients)
        {
            const bool force_replicate = (m_known_clients.find(client.first) == m_known_clients.end());

            BatchedMessageSender batch_sender(client.first, m_message_queue);
            ReplicateSpawns(batch_sender, update_context);

            const int replicated_transforms =
                ReplicateTransforms(transforms_to_replicate, spawns_this_frame, force_replicate, batch_sender, client.second.viewport, update_context);
            const int replicated_sprites =
                ReplicateSprites(sprites_to_replicate, spawns_this_frame, force_replicate, batch_sender, update_context);
            const int replicated_damages =
                ReplicateDamageInfos(damage_info_to_replicate, spawns_this_frame, force_replicate, batch_sender, update_context);
        
            System::Log(
                "replications, transforms: %u, sprites: %u, damages: %u",
                replicated_transforms,
                replicated_sprites,
                replicated_damages);

            known_clients.insert(client.first);
        }

        m_known_clients = known_clients;
    }

    while(!m_message_queue.empty())
    {
        m_server_manager->SendMessage(m_message_queue.front());
        m_message_queue.pop();
    }
}

void ServerReplicator::ReplicateSpawns(BatchedMessageSender& batched_sender, const mono::UpdateContext& update_context)
{
    for(const mono::IEntityManager::SpawnEvent& spawn_event : m_entity_system->GetSpawnEvents())
    {
        SpawnMessage spawn_message;
        spawn_message.timestamp = update_context.timestamp;
        spawn_message.entity_id = spawn_event.entity_id;
        spawn_message.spawn = spawn_event.spawned;

        batched_sender.SendMessage(spawn_message);
    }
}

int ServerReplicator::ReplicateTransforms(
    const std::vector<uint32_t>& entities,
    const std::vector<uint32_t>& spawn_entities,
    bool force_replicate,
    BatchedMessageSender& batched_sender,
    const math::Quad& client_viewport,
    const mono::UpdateContext& update_context)
{
    int replicated_transforms = 0;

    const auto transform_func = [&, this](const math::Matrix& transform, uint32_t id) {

        TransformData& last_transform_message = m_transform_data[id];
        last_transform_message.time_to_replicate -= update_context.delta_ms;

        const bool time_to_replicate = (last_transform_message.time_to_replicate < 0);
        const bool spawned_this_frame = mono::contains(spawn_entities, id);

        if(!time_to_replicate && !force_replicate && !spawned_this_frame)
            return;

        if(!force_replicate && !spawned_this_frame)
        {
             // Expand by 5 meter to send positions before in sight
            const math::Quad& client_bb = math::ResizeQuad(client_viewport, 5.0f);
            const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(id);

            const bool overlaps = math::QuadOverlaps(client_bb, world_bb);
            if(!overlaps)
                return;
        }

        TransformMessage transform_message;
        transform_message.timestamp = update_context.timestamp;
        transform_message.entity_id = id;
        transform_message.parent_transform = m_transform_system->GetParent(id);
        transform_message.position = math::GetPosition(transform);
        transform_message.rotation = math::GetZRotation(transform);

        const bool same_as_last_time = 
            math::IsPrettyMuchEquals(last_transform_message.position, transform_message.position, 0.001f) &&
            math::IsPrettyMuchEquals(last_transform_message.rotation, transform_message.rotation, 0.001f) &&
            last_transform_message.parent_transform == transform_message.parent_transform;

        if(!same_as_last_time || spawned_this_frame || force_replicate)
        {
            batched_sender.SendMessage(transform_message);

            last_transform_message.position = transform_message.position;
            last_transform_message.rotation = transform_message.rotation;
            last_transform_message.parent_transform = transform_message.parent_transform;
            last_transform_message.time_to_replicate = m_replication_interval;

            replicated_transforms++;
       }
    };

    for(uint32_t entity_id : entities)
        transform_func(m_transform_system->GetTransform(entity_id), entity_id);

    return replicated_transforms;
}

int ServerReplicator::ReplicateSprites(
    const std::vector<uint32_t>& entities,
    const std::vector<uint32_t>& spawn_entities,
    bool force_replicate,
    BatchedMessageSender& batched_sender,
    const mono::UpdateContext& update_context)
{
    int replicated_sprites = 0;

    const auto sprite_func = [&, this](mono::ISprite* sprite, uint32_t id) {

        SpriteMessage sprite_message;
        sprite_message.entity_id = id;
        sprite_message.filename_hash = sprite->GetSpriteHash();
        sprite_message.hex_color = mono::Color::ToHex(sprite->GetShade());
        sprite_message.animation_id = sprite->GetActiveAnimation();
        
        sprite_message.properties = sprite->GetProperties();
        sprite_message.layer = 0; //m_sprite_system->GetSpriteLayer(id);
        sprite_message.shadow_size = sprite->GetShadowSize();

        const math::Vector shadow_offset = sprite->GetShadowOffset();
        sprite_message.shadow_offset_x = shadow_offset.x;
        sprite_message.shadow_offset_y = shadow_offset.y;

        SpriteData& last_sprite_data = m_sprite_data[id];

        const bool same_as_last_time =
            last_sprite_data.animation_id == sprite_message.animation_id &&
            last_sprite_data.filename_hash == sprite_message.filename_hash &&
            last_sprite_data.hex_color == sprite_message.hex_color &&
            last_sprite_data.properties == sprite_message.properties;
        const bool spawned_this_frame = mono::contains(spawn_entities, id);

        if(!same_as_last_time || spawned_this_frame || force_replicate)
        {
            batched_sender.SendMessage(sprite_message);
            
            last_sprite_data.animation_id = sprite_message.animation_id;
            last_sprite_data.filename_hash = sprite_message.filename_hash;
            last_sprite_data.hex_color = sprite_message.hex_color;
            last_sprite_data.properties = sprite_message.properties;

            replicated_sprites++;
        }
    };

    for(uint32_t entity_id : entities)
        sprite_func(m_sprite_system->GetSprite(entity_id), entity_id);

    return replicated_sprites;
}

int ServerReplicator::ReplicateDamageInfos(
    const std::vector<uint32_t>& entities,
    const std::vector<uint32_t>& spawn_entities,
    bool force_replicate,
    BatchedMessageSender& batch_sender,
    const mono::UpdateContext& update_context)
{
    int replicated_damages = 0;

    const auto damage_info_func = [&, this](const DamageRecord* damage_record, uint32_t entity_id) {

        HealthData& last_health = m_health_data[entity_id];

        const bool same_as_last_time = (last_health.health == damage_record->health);
        const bool spawned_this_frame = mono::contains(spawn_entities, entity_id);

        if(same_as_last_time && !spawned_this_frame && !force_replicate)
            return;

        last_health.health = damage_record->health;

        DamageInfoMessage damage_info;
        damage_info.entity_id = entity_id;
        damage_info.health = damage_record->health;
        damage_info.full_health = damage_record->full_health;
        damage_info.damage_timestamp = damage_record->last_damaged_timestamp;
        damage_info.is_boss = damage_record->is_boss;

        batch_sender.SendMessage(damage_info);

        replicated_damages++;
    };

    for(uint32_t entity_id : entities)
        damage_info_func(m_damage_system->GetDamageRecord(entity_id), entity_id);

    return replicated_damages;
}
