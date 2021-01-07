
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include <vector>

namespace game
{
    class ClientManager;
    class DamageSystem;
    class PositionPredictionSystem;
    struct SpawnMessage;

    class SpawnPredictionSystem : public mono::IGameSystem
    {
    public:

        SpawnPredictionSystem(
            const ClientManager* client_manager,
            mono::SpriteSystem* sprite_system,
            game::DamageSystem* damage_system,
            game::PositionPredictionSystem* position_prediciton_system);
        void HandleSpawnMessage(const SpawnMessage& spawn_message);

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        const ClientManager* m_client_manager;
        mono::SpriteSystem* m_sprite_system;
        game::DamageSystem* m_damage_system;
        game::PositionPredictionSystem* m_position_prediciton_system;

        std::vector<SpawnMessage> m_spawn_messages;
    };
}
