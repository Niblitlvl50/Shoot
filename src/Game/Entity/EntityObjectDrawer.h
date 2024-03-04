
#pragma once

#include "Rendering/IDrawable.h"

namespace game
{
    class EntityLogicSystem;

    class EntityObjectDrawer : public mono::IDrawable
    {
    public:

        EntityObjectDrawer(game::EntityLogicSystem* entity_logic_system);
    
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        game::EntityLogicSystem* m_entity_logic_system;
    };
}
