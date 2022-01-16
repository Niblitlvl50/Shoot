
#pragma once

#include "MonoFwd.h"
#include "EventHandler/EventToken.h"
#include "IUpdatable.h"
#include "NetworkMessage.h"
#include "NetworkSerialize.h"

#include <queue>
#include <unordered_set>

namespace game
{
    struct LevelMetadata;
}

namespace game
{
    class INetworkPipe;
    class ServerManager;
    class BatchedMessageSender;
    class DamageSystem;
    struct PlayerConnectedEvent;

    class ServerReplicator : public mono::IUpdatable
    {
    public:

        ServerReplicator(
            mono::EventHandler* event_handler,
            mono::EntitySystem* entity_system,
            mono::TransformSystem* transform_system,
            mono::SpriteSystem* sprite_system,
            DamageSystem* damage_system,
            ServerManager* server_manager,
            const game::LevelMetadata& level_metadata,
            uint32_t replication_interval);
        ~ServerReplicator();

    private:
        void Update(const mono::UpdateContext& update_context) override;

        void ReplicateSpawns(BatchedMessageSender& batched_sender, const mono::UpdateContext& update_context);
        int ReplicateTransforms(
            const std::vector<uint32_t>& entities,
            const std::vector<uint32_t>& spawn_entities,
            bool force_replicate,
            BatchedMessageSender& batched_sender,
            const math::Quad& client_viewport,
            const mono::UpdateContext& update_context);
        int ReplicateSprites(
            const std::vector<uint32_t>& entities,
            const std::vector<uint32_t>& spawn_entities,
            bool force_replicate,
            BatchedMessageSender& batched_sender,
            const mono::UpdateContext& update_context);
        int ReplicateDamageInfos(
            const std::vector<uint32_t>& entities,
            const std::vector<uint32_t>& spawn_entities,
            bool force_replicate,
            BatchedMessageSender& batch_sender,
            const mono::UpdateContext& update_context);

        mono::EventHandler* m_event_handler;
        mono::EntitySystem* m_entity_system;
        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        DamageSystem* m_damage_system;
        ServerManager* m_server_manager;
        uint32_t m_replication_interval;

        mono::EventToken<PlayerConnectedEvent> m_connected_token;

        struct TransformData
        {
            int time_to_replicate;
            math::Vector position;
            float rotation;
            uint16_t parent_transform;
        } m_transform_data[500];

        struct SpriteData
        {
            uint32_t filename_hash;
            uint32_t hex_color;
            uint32_t properties;
            short animation_id;
        } m_sprite_data[500];

        struct HealthData
        {
            int health;
        } m_health_data[500];

        std::queue<NetworkMessage> m_message_queue;
        std::unordered_set<network::Address> m_known_clients;
    };
}
