
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
#include "Util/Hash.h"

#include "EntitySystem/IEntityManager.h"
#include "Entity/EntityProperties.h"

#include "Events/GameEventFuncFwd.h"
#include "Events/PlayerConnectedEvent.h"
#include "ScopedTimer.h"
#include "AIKnowledge.h"
#include "WorldFile.h"

#include "Math/MathFunctions.h"
#include "Camera/ICamera.h"

#include <unordered_set>

using namespace game;

static constexpr uint32_t KEYFRAME_INTERVAL = 10;

ServerReplicator::ServerReplicator(
    mono::EventHandler* event_handler,
    mono::EntitySystem* entity_system,
    mono::TransformSystem* transform_system,
    mono::SpriteSystem* sprite_system,
    ServerManager* server_manager,
    const shared::LevelMetadata& level_metadata,
    uint32_t replication_interval)
    : m_event_handler(event_handler)
    , m_entity_system(entity_system)
    , m_transform_system(transform_system)
    , m_sprite_system(sprite_system)
    , m_server_manager(server_manager)
    , m_replication_interval(replication_interval)
{
    std::memset(&m_transform_data, 0, std::size(m_transform_data) * sizeof(TransformData));
    std::memset(&m_sprite_data, 0, std::size(m_sprite_data) * sizeof(SpriteData));

    m_keyframe_low = 0;
    m_keyframe_high = KEYFRAME_INTERVAL;

    const PlayerConnectedFunc connected_func = [server_manager, level_metadata](const PlayerConnectedEvent& event) {

        LevelMetadataMessage metadata_message;
        metadata_message.camera_position = level_metadata.camera_position;
        metadata_message.camera_size = level_metadata.camera_size;
        metadata_message.background_texture_hash = mono::Hash(level_metadata.background_texture.c_str());
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

    std::vector<uint32_t> entity_ids;

    const auto collect_entities = [&entity_ids](const mono::Entity& entity) {
        entity_ids.push_back(entity.id);
    };
    m_entity_system->ForEachEntity(collect_entities);

    const std::unordered_map<network::Address, ClientData>& clients = m_server_manager->GetConnectedClients();
    for(const auto& client : clients)
    {
        BatchedMessageSender batch_sender(client.first, m_message_queue);

        ReplicateSpawns(batch_sender, update_context);
        ReplicateTransforms(entity_ids, batch_sender, client.second.viewport, update_context);
        ReplicateSprites(entity_ids, batch_sender, update_context);
    }

    for(int index = 0; index < 5 && !m_message_queue.empty(); ++index)
    {
        m_server_manager->SendMessage(m_message_queue.front());
        m_message_queue.pop();
    }

    m_keyframe_low += KEYFRAME_INTERVAL;
    m_keyframe_high += KEYFRAME_INTERVAL;

    m_keyframe_high = std::min(m_keyframe_high, 500u);

    if(m_keyframe_low >= 500)
    {
        m_keyframe_low = 0;
        m_keyframe_high = KEYFRAME_INTERVAL;
    }

    if(!m_message_queue.empty())
        System::Log("ServerReplicator|Message queue is not empty, size: %u\n", m_message_queue.size());
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

void ServerReplicator::ReplicateTransforms(
    const std::vector<uint32_t>& entities, BatchedMessageSender& batched_sender, const math::Quad& client_viewport, const mono::UpdateContext& update_context)
{
    int total_transforms = 0;
    int replicated_transforms = 0;

    const auto transform_func = [&, this](const math::Matrix& transform, uint32_t id) {

        total_transforms++;

        TransformData& last_transform_message = m_transform_data[id];
        last_transform_message.time_to_replicate -= update_context.delta_ms;

        const bool keyframe = (id >= m_keyframe_low && id < m_keyframe_high);
        const bool time_to_replicate = (last_transform_message.time_to_replicate < 0);

        if(!time_to_replicate && !keyframe)
            return;

        if(!keyframe)
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
        transform_message.settled = 
            math::IsPrettyMuchEquals(last_transform_message.position, transform_message.position, 0.001f) &&
            math::IsPrettyMuchEquals(last_transform_message.rotation, transform_message.rotation, 0.001f) &&
            last_transform_message.parent_transform == transform_message.parent_transform;

        const bool same_as_last_time =
            transform_message.settled && (last_transform_message.settled == transform_message.settled);

        if(!same_as_last_time || keyframe)
        {
            batched_sender.SendMessage(transform_message);

            last_transform_message.position = transform_message.position;
            last_transform_message.rotation = transform_message.rotation;
            last_transform_message.parent_transform = transform_message.parent_transform;
            last_transform_message.settled = transform_message.settled;
            last_transform_message.time_to_replicate = m_replication_interval;

            replicated_transforms++;
       }
    };

    for(uint32_t entity_id : entities)
        transform_func(m_transform_system->GetTransform(entity_id), entity_id);

    //System::Log(
    //    "keyframe %u - %u, transforms %d/%d\n", m_keyframe_low, m_keyframe_high, replicated_transforms, total_transforms);
}

void ServerReplicator::ReplicateSprites(
    const std::vector<uint32_t>& entities, BatchedMessageSender& batched_sender, const mono::UpdateContext& update_context)
{
    int total_sprites = 0;
    int replicated_sprites = 0;

    const auto sprite_func = [&, this](mono::ISprite* sprite, uint32_t id) {

        SpriteMessage sprite_message;
        sprite_message.entity_id = id;
        sprite_message.filename_hash = sprite->GetSpriteHash();
        sprite_message.hex_color = mono::Color::ToHex(sprite->GetShade());
        sprite_message.vertical_direction = (short)sprite->GetVerticalDirection();
        sprite_message.horizontal_direction = (short)sprite->GetHorizontalDirection();
        sprite_message.animation_id = sprite->GetActiveAnimation();
        
        sprite_message.properties = sprite->GetProperties();
        sprite_message.layer = m_sprite_system->GetSpriteLayer(id);
        sprite_message.shadow_size = sprite->GetShadowSize();

        const math::Vector shadow_offset = sprite->GetShadowOffset();
        sprite_message.shadow_offset_x = shadow_offset.x;
        sprite_message.shadow_offset_y = shadow_offset.y;

        total_sprites++;

        SpriteData& last_sprite_data = m_sprite_data[id];
        last_sprite_data.time_to_replicate -= update_context.delta_ms;

        const bool time_to_replicate = (last_sprite_data.time_to_replicate < 0);
        const bool same =
            last_sprite_data.animation_id == sprite_message.animation_id &&
            last_sprite_data.filename_hash == sprite_message.filename_hash &&
            last_sprite_data.hex_color == sprite_message.hex_color &&
            last_sprite_data.vertical_direction == sprite_message.vertical_direction &&
            last_sprite_data.horizontal_direction == sprite_message.horizontal_direction;
        const bool keyframe = (id >= m_keyframe_low && id < m_keyframe_high);

        if((time_to_replicate && !same) || keyframe)
        {
            batched_sender.SendMessage(sprite_message);
            
            last_sprite_data.animation_id = sprite_message.animation_id;
            last_sprite_data.filename_hash = sprite_message.filename_hash;
            last_sprite_data.hex_color = sprite_message.hex_color;
            last_sprite_data.vertical_direction = sprite_message.vertical_direction;
            last_sprite_data.horizontal_direction = sprite_message.horizontal_direction;
            last_sprite_data.time_to_replicate = m_replication_interval;

            replicated_sprites++;
        }
    };

    for(uint32_t entity_id : entities)
        sprite_func(m_sprite_system->GetSprite(entity_id), entity_id);

//    System::Log(
//        "keyframe %u - %u, transforms %d/%d, sprites %d/%d\n", m_keyframe_low, m_keyframe_high, replicated_transforms, total_transforms, replicated_sprites, total_sprites);
}
