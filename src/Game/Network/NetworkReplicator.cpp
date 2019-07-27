
#include "NetworkReplicator.h"
#include "INetworkPipe.h"
#include "NetworkMessage.h"
#include "BatchedMessageSender.h"

#include "EntitySystem.h"
#include "TransformSystem.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteSystem.h"

#include "Entity/IEntityManager.h"
#include "Entity/EntityProperties.h"

#include "ScopedTimer.h"
#include "AIKnowledge.h"

#include "Math/MathFunctions.h"

#include <unordered_set>

using namespace game;

NetworkReplicator::NetworkReplicator(
    mono::TransformSystem* transform_system,
    mono::SpriteSystem* sprite_system,
    IEntityManager* entity_manager,
    INetworkPipe* remote_connection,
    uint32_t replication_interval)
    : m_transform_system(transform_system)
    , m_sprite_system(sprite_system)
    , m_entity_manager(entity_manager)
    , m_remote_connection(remote_connection)
    , m_replication_interval(replication_interval)
{
    std::memset(&m_transform_data, 0, std::size(m_transform_data) * sizeof(TransformData));
    std::memset(&m_sprite_data, 0, std::size(m_sprite_data) * sizeof(SpriteData));

    const float replication_delta = float(replication_interval) / 500.0f;

    for(int index = 0; index < 500; ++index)
    {
        TransformData& transform_data = m_transform_data[index];
        transform_data.time_to_replicate = replication_delta * index;

        SpriteData& sprite_data = m_sprite_data[index];
        sprite_data.time_to_replicate = replication_delta * index;
    }

    m_keyframe_index = 0;
}

void NetworkReplicator::doUpdate(const mono::UpdateContext& update_context)
{
    //SCOPED_TIMER_AUTO();

    m_keyframe_index++;
    if(m_keyframe_index > 500)
        m_keyframe_index = 0;

    int total_transforms = 0;
    int replicated_transforms = 0;

    int total_sprites = 0;
    int replicated_sprites = 0;

    BatchedMessageSender batch_sender(m_message_queue);

    for(const IEntityManager::SpawnEvent& spawn_event : m_entity_manager->GetSpawnEvents())
    {
        SpawnMessage spawn_message;
        spawn_message.entity_id = spawn_event.entity_id;
        spawn_message.spawn = spawn_event.spawned;

        batch_sender.SendMessage(spawn_message);
    }

    const auto transform_func =
        [this, &total_transforms, &replicated_transforms, &batch_sender, &update_context](const math::Matrix& transform, uint32_t id) {

        TransformMessage transform_message;
        transform_message.timestamp = update_context.total_time;
        transform_message.entity_id = id;
        transform_message.position = math::GetPosition(transform);
        transform_message.rotation = math::GetZRotation(transform);

        total_transforms++;

        TransformData& last_transform_message = m_transform_data[id];
        last_transform_message.time_to_replicate -= update_context.delta_ms;

        const bool time_to_replicate = (last_transform_message.time_to_replicate < 0);
        const bool same =
            math::IsPrettyMuchEquals(last_transform_message.position, transform_message.position, 0.001f) &&
            math::IsPrettyMuchEquals(last_transform_message.rotation, transform_message.rotation, 0.001f);
        const bool keyframe = (m_keyframe_index == id);

        if((time_to_replicate && !same) || keyframe)
        {
            batch_sender.SendMessage(transform_message);

            last_transform_message.position = transform_message.position;
            last_transform_message.rotation = transform_message.rotation;
            last_transform_message.time_to_replicate = m_replication_interval;

            replicated_transforms++;
       }
    };

    m_transform_system->ForEachTransform(transform_func);

    const auto sprite_func =
        [this, &total_sprites, &replicated_sprites, &batch_sender, &update_context](mono::ISprite* sprite, uint32_t id) {

        SpriteMessage sprite_message;
        sprite_message.entity_id = id;
        sprite_message.filename_hash = sprite->GetSpriteHash();
        sprite_message.hex_color = mono::Color::ToHex(sprite->GetShade());
        sprite_message.vertical_direction = (short)sprite->GetVerticalDirection();
        sprite_message.horizontal_direction = (short)sprite->GetHorizontalDirection();
        sprite_message.animation_id = sprite->GetActiveAnimation();

        total_sprites++;

        SpriteData& last_sprite_data = m_sprite_data[id];
        last_sprite_data.time_to_replicate -= update_context.delta_ms;

        const bool time_to_replicate = (last_sprite_data.time_to_replicate < 0);
        const bool same =
            last_sprite_data.animation_id == sprite_message.animation_id &&
            last_sprite_data.hex_color == sprite_message.hex_color &&
            last_sprite_data.vertical_direction == sprite_message.vertical_direction &&
            last_sprite_data.horizontal_direction == sprite_message.horizontal_direction;
        const bool keyframe = (m_keyframe_index == id);

        if((time_to_replicate && !same) || keyframe)
        {
            batch_sender.SendMessage(sprite_message);
            
            last_sprite_data.animation_id = sprite_message.animation_id;
            last_sprite_data.hex_color = sprite_message.hex_color;
            last_sprite_data.vertical_direction = sprite_message.vertical_direction;
            last_sprite_data.horizontal_direction = sprite_message.horizontal_direction;
            last_sprite_data.time_to_replicate = m_replication_interval;

            replicated_sprites++;
        }
    };

    m_sprite_system->ForEachSprite(sprite_func);

    for(int index = 0; index < 5 && !m_message_queue.empty(); ++index)
    {
        m_remote_connection->SendMessage(m_message_queue.front());
        m_message_queue.pop();
    }

    std::printf(
        "keyframe %u, transforms %d/%d, sprites %d/%d\n", m_keyframe_index, replicated_transforms, total_transforms, replicated_sprites, total_sprites);
}


ClientReplicator::ClientReplicator(INetworkPipe* remote_connection)
    : m_remote_connection(remote_connection)
{ }

void ClientReplicator::doUpdate(const mono::UpdateContext& update_context)
{
    if(game::g_player_one.is_active)
    {
        RemoteInputMessage remote_input;
        remote_input.controller_state = System::GetController(System::ControllerId::Primary);

        NetworkMessage message;
        message.payload = SerializeMessage(remote_input);
        m_remote_connection->SendMessage(message);
    }
}
