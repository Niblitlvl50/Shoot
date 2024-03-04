
#include "EntityObjectDrawer.h"
#include "EntityLogicSystem.h"
#include "IEntityLogic.h"

#include "Math/Quad.h"

using namespace game;

EntityObjectDrawer::EntityObjectDrawer(game::EntityLogicSystem* entity_logic_system)
    : m_entity_logic_system(entity_logic_system)
{ }

void EntityObjectDrawer::Draw(mono::IRenderer& renderer) const
{
    const auto callback = [&renderer](uint32_t id, EntityLogicComponent& logic) {
        logic.logic->Draw(renderer);
    };
    m_entity_logic_system->ForEach(callback);
}

math::Quad EntityObjectDrawer::BoundingBox() const
{
    return math::InfQuad;
}
