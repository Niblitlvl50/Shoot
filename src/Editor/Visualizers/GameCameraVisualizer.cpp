
#include "GameCameraVisualizer.h"
#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "WorldFile.h"

#include "Navigation/NavmeshFactory.h"

using namespace editor;

GameCameraVisualizer::GameCameraVisualizer(const bool& enabled, const game::LevelMetadata& metadata)
    : m_enabled(enabled)
    , m_metadata(metadata)
{ }

void GameCameraVisualizer::Draw(mono::IRenderer& renderer) const
{
    if(!m_enabled)
        return;

    // Camera
    {
        const math::Vector half_size = m_metadata.camera_size / 2.0f;
        renderer.DrawQuad(math::Quad(m_metadata.camera_position - half_size, m_metadata.camera_position + half_size), mono::Color::RED, 2.0f);
        renderer.DrawPoints({ m_metadata.player_spawn_point }, mono::Color::CYAN, 10.0f);
    }

    {
        if(m_metadata.use_package_spawn_position)
            renderer.DrawPoints({ m_metadata.package_spawn_position }, mono::Color::MAGENTA, 10.0f);
    }

    // Navmesh
    {
        const math::Quad navmesh_quad = { m_metadata.navmesh_start, m_metadata.navmesh_end };
        renderer.DrawQuad(navmesh_quad, mono::Color::GREEN, 2.0f);

        const std::vector<math::Vector>& points = game::GenerateMeshPoints(m_metadata.navmesh_start, m_metadata.navmesh_end, m_metadata.navmesh_density);
        renderer.DrawPoints(points, mono::Color::GREEN, 2.0f);
    }
}

math::Quad GameCameraVisualizer::BoundingBox() const
{
    return math::InfQuad;
}
