
#include "WorldBoundsSystem.h"
#include "System/Hash.h"

using namespace game;

WorldBoundsSystem::WorldBoundsSystem()
{

}

void WorldBoundsSystem::AddPolygon(const std::vector<math::Vector>& vertices, const std::string& texture_file)
{
    
}

uint32_t WorldBoundsSystem::Id() const
{
    return hash::Hash(Name());
}

const char* WorldBoundsSystem::Name() const
{
    return "world_bounds_system";
}

void WorldBoundsSystem::Update(const mono::UpdateContext& update_context)
{

}

