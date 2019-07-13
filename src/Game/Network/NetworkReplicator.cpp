
#include "NetworkReplicator.h"
#include "INetworkPipe.h"
#include "NetworkMessage.h"
#include "BatchedMessageSender.h"

#include "TransformSystem.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteSystem.h"

#include "Entity/IEntityManager.h"

#include "ScopedTimer.h"
#include "AIKnowledge.h"

using namespace game;

NetworkReplicator::NetworkReplicator(
    mono::TransformSystem* transform_system, mono::SpriteSystem* sprite_system, IEntityManager* entity_manager, INetworkPipe* remote_connection)
    : m_transform_system(transform_system)
    , m_sprite_system(sprite_system)
    , m_entity_manager(entity_manager)
    , m_remote_connection(remote_connection)
{
    std::memset(&m_transform_messages, 0, std::size(m_transform_messages) * sizeof(TransformMessage));
    std::memset(&m_sprite_messages, 0, std::size(m_sprite_messages) * sizeof(SpriteMessage));
}

void NetworkReplicator::doUpdate(const mono::UpdateContext& update_context)
{
    //SCOPED_TIMER_AUTO();

    BatchedMessageSender batch_sender(m_remote_connection);

    for(const IEntityManager::SpawnEvent& spawn_event : m_entity_manager->GetSpawnEvents())
    {
        SpawnMessage spawn_message;
        spawn_message.entity_id = spawn_event.entity_id;
        spawn_message.spawn = spawn_event.spawned;

        batch_sender.SendMessage(spawn_message);
    }

    const auto transform_func = [this, &batch_sender](const math::Matrix& transform, uint32_t id) {
        TransformMessage transform_message;
        transform_message.entity_id = id;
        transform_message.position = math::GetPosition(transform);
        transform_message.rotation = math::GetZRotation(transform);

        const bool same = std::memcmp(&m_transform_messages[id], &transform_message, sizeof(TransformMessage)) == 0;
        if(!same)
        {
            batch_sender.SendMessage(transform_message);
            m_transform_messages[id] = transform_message;
        }
    };

    m_transform_system->ForEachTransform(transform_func);

    const auto sprite_func = [this, &batch_sender](mono::ISprite* sprite, uint32_t id) {
        SpriteMessage sprite_message;
        sprite_message.entity_id = id;
        sprite_message.filename_hash = sprite->GetSpriteHash();
        sprite_message.hex_color = mono::Color::ToHex(sprite->GetShade());
        sprite_message.vertical_direction = (int)sprite->GetVerticalDirection();
        sprite_message.horizontal_direction = (int)sprite->GetHorizontalDirection();
        sprite_message.animation_id = sprite->GetActiveAnimation();

        const bool same = std::memcmp(&m_sprite_messages[id], &sprite_message, sizeof(SpriteMessage)) == 0;
        if(!same)
        {
            batch_sender.SendMessage(sprite_message);
            m_sprite_messages[id] = sprite_message;
        }
    };

    m_sprite_system->RunForEachSprite(sprite_func, m_remote_connection);
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
