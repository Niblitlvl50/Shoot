
#include "WorldBoundsSystem.h"
#include "System/Hash.h"
#include "System/System.h"

using namespace game;

WorldBoundsSystem::WorldBoundsSystem()
{

}

void WorldBoundsSystem::AllocateTexturedPolygon(uint32_t id)
{
    System::Log("Allocate %u", id);
}

void WorldBoundsSystem::ReleaseTexturedPolygon(uint32_t id)
{
    System::Log("Release %u", id);
}

void WorldBoundsSystem::AddPolygon(uint32_t id, const std::vector<math::Vector>& vertices, const std::string& texture_file)
{
    System::Log("Add Polygon to %u", id);
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

