
#pragma once

#include "IUpdatable.h"
#include "NetworkMessage.h"
#include "NetworkSerialize.h"

#include <queue>

class IEntityManager;

namespace mono
{
    class EntitySystem;
    class TransformSystem;
    class PhysicsSystem;
    class SpriteSystem;
    class ICamera;
}

namespace game
{
    class INetworkPipe;
    class ServerManager;
    class BatchedMessageSender;

    class ServerReplicator : public mono::IUpdatable
    {
    public:

        ServerReplicator(
            mono::EntitySystem* entity_system,
            mono::TransformSystem* transform_system,
            mono::SpriteSystem* sprite_system,
            IEntityManager* entity_manager,
            ServerManager* server_manager,
            uint32_t replication_interval);

    private:
        void doUpdate(const mono::UpdateContext& update_context) override;

        void ReplicateSpawns(BatchedMessageSender& batched_sender);
        void ReplicateTransforms(const std::vector<uint32_t>& entities, BatchedMessageSender& batched_sender, const math::Quad& client_viewport, const mono::UpdateContext& update_context);
        void ReplicateSprites(const std::vector<uint32_t>& entities, BatchedMessageSender& batched_sender, const mono::UpdateContext& update_context);

        mono::EntitySystem* m_entity_system;
        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        IEntityManager* m_entity_manager;
        ServerManager* m_server_manager;
        uint32_t m_replication_interval;

        uint32_t m_keyframe_low;
        uint32_t m_keyframe_high;

        struct TransformData
        {
            int time_to_replicate;
            math::Vector position;
            float rotation;
            bool settled;
        } m_transform_data[500];

        struct SpriteData
        {
            int time_to_replicate;
            uint32_t hex_color;
            short animation_id;
            bool vertical_direction;
            bool horizontal_direction;
        } m_sprite_data[500];

        std::queue<NetworkMessage> m_message_queue;
    };
}
