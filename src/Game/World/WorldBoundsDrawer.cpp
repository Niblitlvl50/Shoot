
#include "WorldBoundsDrawer.h"
#include "WorldBoundsSystem.h"

#include "TransformSystem/TransformSystem.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Rendering/RenderBuffer/BufferFactory.h"
#include "Rendering/Texture/ITexture.h"
#include "Math/Quad.h"
#include "Math/MathFunctions.h"

#include "Rendering/RenderSystem.h"

using namespace game;

WorldBoundsDrawer::WorldBoundsDrawer(const mono::TransformSystem* transform_system, const WorldBoundsSystem* world_system)
    : m_transform_system(transform_system)
    , m_world_system(world_system)
{
    m_texture = mono::GetTextureFactory()->CreateTexture("res/textures/noise/noise_1.png");
}

void WorldBoundsDrawer::Draw(mono::IRenderer& renderer) const
{
    const auto draw_world_bounds = [this, &renderer](const WorldBoundsComponent& component) {

        const bool no_triangles = component.triangulated_points.triangles.empty();
        if(no_triangles)
            return;

        mono::ITexture* texture = component.texture.get();
        if(!texture)
            return;

        const math::Quad world_bb = m_transform_system->GetWorldBoundingBox(component.id);
        if(!renderer.Cull(world_bb))
            return;

        BuildBuffers(component);
        const InternalRenderData& render_data = m_id_to_buffers[component.id];
        //renderer.DrawTrianges(
        //    render_data.vertices.get(), render_data.colors.get(), render_data.indices.get(), 0, render_data.indices->Size());
        //renderer.DrawFog(render_data.vertices.get(), render_data.indices.get(), m_texture.get());

        const math::Matrix world_transform = renderer.GetTransform() * m_transform_system->GetWorld(component.id);
        const auto transform_scope = mono::MakeTransformScope(world_transform, &renderer);

        renderer.DrawGeometry(
            render_data.vertices.get(),
            render_data.uvs.get(),
            render_data.indices.get(),
            texture,
            component.color_tint,
            false,
            render_data.indices->Size());
    };
    m_world_system->ForEachComponent(draw_world_bounds);
}

math::Quad WorldBoundsDrawer::BoundingBox() const
{
    return math::InfQuad;
}

void WorldBoundsDrawer::BuildBuffers(const WorldBoundsComponent& component) const
{
    const auto it = m_id_to_buffers.find(component.id);
    if(it != m_id_to_buffers.end())
    {
        const bool not_updated = (it->second.timestamp == component.timestamp);
        if(not_updated)
            return;
    }



    const uint32_t n_vertices = component.triangulated_points.vertices.size();
    const uint32_t n_indices = component.triangulated_points.triangles.size();

    const std::vector<mono::Color::RGBA> colors(n_vertices, mono::Color::CYAN);
    const math::Quad local_bb = m_transform_system->GetBoundingBox(component.id);

    math::Vector repeate = math::Vector(1.0f, 1.0f);
    
    {
        const float pixels_per_meter = mono::PixelsPerMeter();
        const float background_width = math::Width(local_bb);
        const float background_height = math::Height(local_bb);
        repeate = math::Vector(
            background_width / component.texture->Width(),
            background_height / component.texture->Height()) * pixels_per_meter;
    }


    std::vector<math::Vector> uv_data;
    uv_data.reserve(n_vertices);

    for(const math::Vector& vertex : component.triangulated_points.vertices)
        uv_data.push_back(math::MapVectorInQuad(vertex, local_bb) * repeate);

    InternalRenderData& render_data = m_id_to_buffers[component.id];
    render_data.timestamp = component.timestamp;
    render_data.vertices = mono::CreateRenderBuffer(
        mono::BufferType::STATIC,
        mono::BufferData::FLOAT,
        2,
        n_vertices,
        component.triangulated_points.vertices.data());
    render_data.colors = mono::CreateRenderBuffer(
        mono::BufferType::STATIC,
        mono::BufferData::FLOAT,
        4,
        colors.size(),
        colors.data());
    render_data.uvs = mono::CreateRenderBuffer(
        mono::BufferType::STATIC,
        mono::BufferData::FLOAT,
        2,
        uv_data.size(),
        uv_data.data());
    render_data.indices = mono::CreateElementBuffer(
        mono::BufferType::STATIC,
        n_indices,
        component.triangulated_points.triangles.data());
}
