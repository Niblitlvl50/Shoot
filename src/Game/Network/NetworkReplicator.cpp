
#include "NetworkReplicator.h"
#include "INetworkPipe.h"
#include "NetworkMessage.h"

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
{ }

void NetworkReplicator::doUpdate(const mono::UpdateContext& update_context)
{
    //SCOPED_TIMER_AUTO();

    for(const IEntityManager::SpawnEvent& spawn_event : m_entity_manager->GetSpawnEvents())
    {
        SpawnMessage spawn_message;
        spawn_message.entity_id = spawn_event.entity_id;
        spawn_message.spawn = spawn_event.spawned;

        NetworkMessage message;
        message.payload = SerializeMessage(spawn_message);
        m_remote_connection->SendMessage(message);
    }

    const auto transform_func = [this](const math::Matrix& transform, uint32_t id) {
        TransformMessage transform_message;
        transform_message.entity_id = id;
        transform_message.transform = transform;

        NetworkMessage message;
        message.payload = SerializeMessage(transform_message);
        m_remote_connection->SendMessage(message);
    };

    m_transform_system->ForEachTransform(transform_func);

    const auto sprite_func = [](mono::ISprite* sprite, uint32_t id, void* context) {
        SpriteMessage sprite_message;
        sprite_message.entity_id = id;
        sprite_message.filename_hash = sprite->GetSpriteHash();
        sprite_message.shade = sprite->GetShade();
        sprite_message.vertical_direction = (int)sprite->GetVerticalDirection();
        sprite_message.horizontal_direction = (int)sprite->GetHorizontalDirection();
        sprite_message.animation_id = sprite->GetActiveAnimation();

        INetworkPipe* network_pipe = static_cast<INetworkPipe*>(context);

        NetworkMessage message;
        message.payload = SerializeMessage(sprite_message);
        network_pipe->SendMessage(message);
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
