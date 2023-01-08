
#include "WorldBoundsSystem.h"
#include "System/Hash.h"
#include "System/System.h"
#include "Util/Algorithm.h"
#include "TransformSystem/TransformSystem.h"

#include "Rendering/RenderSystem.h"
#include "Math/Triangulate.h"

#include "ScopedTimer.h"

using namespace game;

WorldBoundsSystem::WorldBoundsSystem(mono::TransformSystem* transform_system)
    : m_transform_system(transform_system)
{
    m_components.reserve(32);
}

void WorldBoundsSystem::AllocateTexturedPolygon(uint32_t id)
{
    WorldBoundsComponent component;
    component.id = id;
    m_components.push_back(component);
}

void WorldBoundsSystem::ReleaseTexturedPolygon(uint32_t id)
{
    const auto find_by_id = [id](const WorldBoundsComponent& component) {
        return component.id == id;
    };
    mono::remove_if(m_components, find_by_id);
}

void WorldBoundsSystem::AddPolygon(
    uint32_t id,
    const std::vector<math::Vector>& vertices,
    const std::string& texture_file,
    const mono::Color::RGBA& color,
    PolygonDrawLayer draw_layer)
{
    const auto find_by_id = [id](const WorldBoundsComponent& component) {
        return component.id == id;
    };

    const auto it = std::find_if(m_components.begin(), m_components.end(), find_by_id);
    if(it == m_components.end())
    {
        System::Log("Unable to find WorldBoundsComponent from id: %u", id);
        return;
    }

    WorldBoundsComponent& component = *it;
    component.id = id;
    component.timestamp = System::GetMilliseconds();
    component.texture = mono::RenderSystem::GetTextureFactory()->CreateTexture(texture_file.c_str());
    component.color_tint = color;
    component.draw_layer = draw_layer;
    component.outline = vertices;
    component.triangulated_points = mono::Triangulate(component.outline);

    math::Quad local_polygon_bounds = math::InverseInfQuad;
    for(const math::Vector& vertex : component.outline)
        math::ExpandBy(local_polygon_bounds, vertex);

    math::Quad& bounding_box = m_transform_system->GetBoundingBox(id);
    bounding_box = local_polygon_bounds;
}

const char* WorldBoundsSystem::Name() const
{
    return "world_bounds_system";
}

void WorldBoundsSystem::Update(const mono::UpdateContext& update_context)
{

}

