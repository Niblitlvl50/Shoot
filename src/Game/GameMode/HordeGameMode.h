
#pragma once

#include "IGameMode.h"
#include "MonoFwd.h"
#include "Math/Vector.h"

#include <memory>

namespace game
{
    class HordeGameMode : public IGameMode
    {
    public:

        HordeGameMode();
        virtual ~HordeGameMode();

        void Begin(
            mono::IZone* zone,
            mono::IRenderer* renderer,
            mono::SystemContext* system_context,
            mono::EventHandler* event_handler,
            const struct LevelMetadata& level_metadata) override;
        int End(mono::IZone* zone) override;
        void Update(const mono::UpdateContext& update_context) override;

        void OnSpawnPlayer(uint32_t player_entity_id, const math::Vector& position);
        void SpawnPackage(const math::Vector& position);

        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        mono::IEntityManager* m_entity_manager;
        class PlayerDaemonSystem* m_player_system;

        std::unique_ptr<class PackageAuxiliaryDrawer> m_package_aux_drawer;
        std::unique_ptr<class PlayerUIElement> m_player_ui;
        std::unique_ptr<class LevelTimerUIElement> m_timer_screen;


        bool m_spawn_package;
        bool m_package_spawned;
        math::Vector m_package_spawn_position;
        int m_next_zone;

        uint32_t m_package_release_callback;
        uint32_t m_package_entity_id;

        float m_level_timer;
    };
}
