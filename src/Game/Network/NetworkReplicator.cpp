
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
    , m_replicate_timer(0)
{
    std::memset(&m_transform_messages, 0, std::size(m_transform_messages) * sizeof(TransformMessage));
    std::memset(&m_sprite_messages, 0, std::size(m_sprite_messages) * sizeof(SpriteMessage));
}

void NetworkReplicator::doUpdate(const mono::UpdateContext& update_context)
{
    //SCOPED_TIMER_AUTO();

    int total_transforms = 0;
    int replicated_transforms = 0;

    int total_sprites = 0;
    int replicated_sprites = 0;

    BatchedMessageSender batch_sender(m_remote_connection);

    for(const IEntityManager::SpawnEvent& spawn_event : m_entity_manager->GetSpawnEvents())
    {
        SpawnMessage spawn_message;
        spawn_message.entity_id = spawn_event.entity_id;
        spawn_message.spawn = spawn_event.spawned;

        batch_sender.SendMessage(spawn_message);
    }

    uint32_t start_id = 0;
    uint32_t end_id = 512;

/*
    if(update_context.frame_count % 2 == 0)
    {
        start_id = 0;
        end_id = 512 / 2;
    }
    else
    {
        start_id = 512 / 2;
        end_id = 512;
    }
 */

    const auto transform_func =
        [this, &total_transforms, &replicated_transforms, &batch_sender, start_id, end_id, &update_context]
            (const math::Matrix& transform, uint32_t id) {

        if(id < start_id || id > end_id)
            return;

        TransformMessage transform_message;
        transform_message.timestamp = update_context.total_time;
        transform_message.entity_id = id;
        transform_message.position = math::GetPosition(transform);
        transform_message.rotation = math::GetZRotation(transform);

        total_transforms++;

        const TransformMessage& last_transform_message = m_transform_messages[id];
        const bool same =
            math::IsPrettyMuchEquals(last_transform_message.position, transform_message.position, 0.001f) &&
            math::IsPrettyMuchEquals(last_transform_message.rotation, transform_message.rotation, 0.001f);
        if(!same || true)
        {
            batch_sender.SendMessage(transform_message);
            m_transform_messages[id] = transform_message;
            replicated_transforms++;
        }
    };

    m_replicate_timer += update_context.delta_ms;
    if(m_replicate_timer > m_replication_interval)
    {
        m_transform_system->ForEachTransform(transform_func);
        m_replicate_timer = 0;
    }

    const auto sprite_func =
        [this, &total_sprites, &replicated_sprites, &batch_sender](mono::ISprite* sprite, uint32_t id) {

        SpriteMessage sprite_message;
        sprite_message.entity_id = id;
        sprite_message.filename_hash = sprite->GetSpriteHash();
        sprite_message.hex_color = mono::Color::ToHex(sprite->GetShade());
        sprite_message.vertical_direction = (short)sprite->GetVerticalDirection();
        sprite_message.horizontal_direction = (short)sprite->GetHorizontalDirection();
        sprite_message.animation_id = sprite->GetActiveAnimation();

        total_sprites++;

        const bool same = std::memcmp(&m_sprite_messages[id], &sprite_message, sizeof(SpriteMessage)) == 0;
        if(!same)
        {
            batch_sender.SendMessage(sprite_message);
            m_sprite_messages[id] = sprite_message;
            replicated_sprites++;
        }
    };

    m_sprite_system->ForEachSprite(sprite_func);

//    std::printf(
//        "transforms %d/%d, sprites %d/%d\n", replicated_transforms, total_transforms, replicated_sprites, total_sprites);
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
