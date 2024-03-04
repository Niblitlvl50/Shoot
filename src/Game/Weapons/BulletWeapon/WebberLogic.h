
#pragma once

#include "MonoFwd.h"
#include "Physics/PhysicsFwd.h"
#include "Entity/IEntityLogic.h"

#include <vector>

namespace game
{
    class WebberLogic : public game::IEntityLogic
    {
    public:

        WebberLogic(
            uint32_t entity_id,
            mono::IEntityManager* entity_system,
            mono::PhysicsSystem* physics_system,            
            mono::TransformSystem* transform_system);

        void Destroy();

        void Update(const mono::UpdateContext& update_context) override;
        void Draw(mono::IRenderer& renderer) const override;
        void AttachTo(mono::IBody* body, const math::Vector& world_position);

    private:

        const uint32_t m_entity_id;
        mono::IEntityManager* m_entity_system;
        mono::PhysicsSystem* m_physics_system;
        mono::TransformSystem* m_transform_system;
        float m_life_timer;

        struct AttachedBody
        {
            mono::IBody* body;
            math::Vector local_offset;
        };
        std::vector<AttachedBody> m_attached_bodies;

        std::vector<mono::IConstraint*> m_constraints;
    };
}
