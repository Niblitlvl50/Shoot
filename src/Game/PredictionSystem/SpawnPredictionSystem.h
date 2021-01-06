
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include <vector>

namespace game
{
    class ClientManager;
    class DamageSystem;
    struct SpawnMessage;

    class SpawnPredictionSystem : public mono::IGameSystem
    {
    public:

        SpawnPredictionSystem(
            const ClientManager* client_manager,
            mono::SpriteSystem* sprite_system,
            game::DamageSystem* damage_system,
            mono::IEntityManager* entity_manager);
        void HandleSpawnMessage(const SpawnMessage& spawn_message);

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        const ClientManager* m_client_manager;
        mono::SpriteSystem* m_sprite_system;
        game::DamageSystem* m_damage_system;
        mono::IEntityManager* m_entity_manager;
        std::vector<SpawnMessage> m_spawn_messages;
    };
}
