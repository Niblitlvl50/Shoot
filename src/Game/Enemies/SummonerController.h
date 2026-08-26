
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"
#include "Entity/IEntityLogic.h"
#include "Entity/TargetTypes.h"
#include "StateMachine.h"
#include "Math/Vector.h"
#include "Behaviour/HomingBehaviour.h"
#include "Behaviour/TrackingBehaviour.h"

#include <cstdint>
#include <string>
#include <vector>

namespace game
{
    class SummonerController : public IEntityLogic
    {
    public:

        SummonerController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);
        void Update(const mono::UpdateContext& update_context) override;
        void DrawDebugInfo(class IDebugDrawer* debug_drawer) const override;
        const char* GetDebugCategory() const override;

    private:

        void ToIdle();
        void Idle(const mono::UpdateContext& update_context);

        void ToPrepare();
        void Prepare(const mono::UpdateContext& update_context);

        void ToSummon();
        void Summon(const mono::UpdateContext& update_context);

        void ToCooldown();
        void Cooldown(const mono::UpdateContext& update_context);

        int CountActiveMinions() const;

        enum class States { IDLE, PREPARE, SUMMON, COOLDOWN };

        uint32_t m_entity_id;
        mono::TransformSystem* m_transform_system;
        mono::IEntityManager* m_entity_manager;
        class NavigationSystem* m_navigation_system;
        class TargetSystem* m_target_system;
        mono::ISprite* m_sprite;
        int m_idle_anim_id;
        int m_walk_anim_id;
        int m_cast_anim_id;

        using SummonerStateMachine = StateMachine<States, const mono::UpdateContext&>;
        SummonerStateMachine m_states;

        std::string m_minion_entity_file;
        int m_max_minions;
        std::vector<uint32_t> m_summoned_ids;

        float m_prepare_timer_s;
        float m_cooldown_timer_s;

        ITargetPtr m_player_target;
        HomingBehaviour m_homing_movement;
        TrackingBehaviour m_tracking_movement;
    };
}
