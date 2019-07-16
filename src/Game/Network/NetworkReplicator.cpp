
#include "NetworkReplicator.h"
#include "INetworkPipe.h"
#include "NetworkMessage.h"
#include "BatchedMessageSender.h"

#include "EntitySystem.h"
#include "TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/IBody.h"
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
    mono::PhysicsSystem* physics_system,
    mono::SpriteSystem* sprite_system,
    IEntityManager* entity_manager,
    INetworkPipe* remote_connection)
    : m_transform_system(transform_system)
    , m_physics_system(physics_system)
    , m_sprite_system(sprite_system)
    , m_entity_manager(entity_manager)
    , m_remote_connection(remote_connection)
{
    std::memset(&m_transform_messages, 0, std::size(m_transform_messages) * sizeof(TransformMessage));
    std::memset(&m_sprite_messages, 0, std::size(m_sprite_messages) * sizeof(SpriteMessage));

    m_replicate_timer = 0;
}

void NetworkReplicator::doUpdate(const mono::UpdateContext& update_context)
{
    //SCOPED_TIMER_AUTO();

    m_replicate_timer += update_context.delta_ms;

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

    const auto transform_func =
        [this, &total_transforms, &replicated_transforms, &batch_sender](const math::Matrix& transform, uint32_t id) {

        TransformMessage transform_message;
        transform_message.entity_id = id;
        transform_message.position = math::GetPosition(transform);
        transform_message.rotation = math::GetZRotation(transform);

        const mono::IBody* body = m_physics_system->GetBody(id);
        if(body)
            transform_message.velocity = body->GetVelocity();

        total_transforms++;

        const TransformMessage& last_transform_message = m_transform_messages[id];
        const bool same =
            math::IsPrettyMuchEquals(last_transform_message.position, transform_message.position, 0.01f) &&
            math::IsPrettyMuchEquals(last_transform_message.velocity, transform_message.velocity, 0.01f) &&
            math::IsPrettyMuchEquals(last_transform_message.rotation, transform_message.rotation, 0.01f);
        if(!same)
        {
            batch_sender.SendMessage(transform_message);
            m_transform_messages[id] = transform_message;
            replicated_transforms++;
        }
    };


    if(m_replicate_timer >= 60)
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
        sprite_message.vertical_direction = (int)sprite->GetVerticalDirection();
        sprite_message.horizontal_direction = (int)sprite->GetHorizontalDirection();
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

    m_sprite_system->RunForEachSprite(sprite_func, m_remote_connection);

    std::printf(
        "transforms %d/%d, sprites %d/%d\n", replicated_transforms, total_transforms, replicated_sprites, total_sprites);
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
