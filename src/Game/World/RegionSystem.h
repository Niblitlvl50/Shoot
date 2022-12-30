
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
    };

    struct RegionComponent
    {
        std::string text;
        std::string sub_text;

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
        void UpdateRegion(uint32_t entity_id, const std::string& text, const std::string& sub_text);

        void ActivateRegion(uint32_t entity_id);
        const RegionDescription& GetActivatedRegion() const;

    private:

        mono::PhysicsSystem* m_physics_system;
        std::unordered_map<uint32_t, RegionComponent> m_regions;
        std::unordered_set<uint32_t> m_activated_regions;
        game::RegionDescription m_active_region;
    };
}
