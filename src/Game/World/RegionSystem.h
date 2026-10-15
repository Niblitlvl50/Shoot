
#pragma once

#include "IGameSystem.h"
#include "Physics/PhysicsFwd.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace game
{
    struct RegionDescription
    {
        uint32_t entity_id;
        std::string text;
        std::string sub_text;
        float duration;

        // Increased on every activation, so a region shown again can be told apart from the last showing.
        uint32_t activation_count = 0;

        // Set when the player leaves the region, the drawer then fades it out right away.
        bool exited = false;
    };

    struct RegionComponent
    {
        std::string text;
        std::string sub_text;
        float duration;

        // Shown on every entry instead of only the first time.
        bool show_every_time;

        // Internal data
        std::unique_ptr<mono::ICollisionHandler> collision_handler;
    };

    class RegionSystem : public mono::IGameSystem
    {
    public:

        RegionSystem(mono::PhysicsSystem* physics_system);

        const char* Name() const override;
        void Reset() override;
        void Update(const mono::UpdateContext& update_context) override;

        void AllocateRegion(uint32_t entity_id);
        void ReleaseRegion(uint32_t entity_id);
        void UpdateRegion(uint32_t entity_id, const std::string& text, const std::string& sub_text, float duration_s, bool show_every_time);

        void ActivateRegion(uint32_t entity_id);
        void ExitRegion(uint32_t entity_id);
        const RegionDescription& GetActivatedRegion() const;

    private:

        mono::PhysicsSystem* m_physics_system;
        std::unordered_map<uint32_t, RegionComponent> m_regions;
        std::unordered_set<uint32_t> m_activated_regions;
        game::RegionDescription m_active_region;
    };
}
