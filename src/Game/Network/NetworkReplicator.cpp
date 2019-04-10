
#include "NetworkReplicator.h"
#include "RemoteConnection.h"
#include "NetworkMessage.h"

#include "Zone/PhysicsZone.h"
#include "Entity/IPhysicsEntity.h"
#include "Math/Vector.h"

#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteSystem.h"

using namespace game;

NetworkReplicator::NetworkReplicator(
    const mono::PhysicsZone* physics_zone, mono::SpriteSystem* sprites, RemoteConnection* remote_connection)
    : m_physics_zone(physics_zone)
    , m_sprite_system(sprites)
    , m_remote_connection(remote_connection)
{ }

void NetworkReplicator::doUpdate(unsigned int delta)
{
    for(const mono::IPhysicsEntityPtr& entity : m_physics_zone->GetPhysicsEntities())
    {
        const math::Vector position = entity->Position();

        PositionalMessage pos_message;
        pos_message.entity_id = entity->Id();
        pos_message.x = position.x;
        pos_message.y = position.y;
        pos_message.rotation = entity->Rotation();

        NetworkMessage message;
        message.id = 123;
        message.payload = SerializeMessage(pos_message);

        m_remote_connection->SendMessage(message);
    }

    const auto sprite_func = [](mono::ISprite* sprite, uint32_t id, void* context) {
        AnimationMessage animation_message;
        animation_message.entity_id = id;
        animation_message.animation_id = sprite->GetActiveAnimation();

        NetworkMessage message;
        message.id = 666;
        message.payload = SerializeMessage(animation_message);

        RemoteConnection* connection = static_cast<RemoteConnection*>(context);
        connection->SendMessage(message);
    };

    m_sprite_system->RunForEachSprite(sprite_func, m_remote_connection);
}
