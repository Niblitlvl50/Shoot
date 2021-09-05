
#include "PathProxy.h"

#include "IObjectVisitor.h"
#include "Grabber.h"
#include "SnapPoint.h"
#include "UI/UIContext.h"
#include "UI/UIProperties.h"
#include "Editor.h"
#include "Math/Matrix.h"
#include "Math/Quad.h"
#include "Math/MathFunctions.h"

#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"

#include "Component.h"

using namespace editor;

PathProxy::PathProxy(
    uint32_t entity_id,
    const std::vector<Component>& components,
    mono::IEntityManager* entity_manager,
    mono::TransformSystem* transform_system,
    Editor* editor)
    : m_entity_id(entity_id)
    , m_components(components)
    , m_entity_manager(entity_manager)
    , m_transform_system(transform_system)
    , m_editor(editor)
{ }

PathProxy::~PathProxy()
{ }

uint32_t PathProxy::Uuid() const
{
    return 0;
}

uint32_t PathProxy::Id() const
{
    return m_entity_id;
}

std::string PathProxy::Name() const
{
    const Component* name_folder_component = FindComponentFromHash(NAME_FOLDER_COMPONENT, m_components);
    if(!name_folder_component)
        return "Unknown";

    std::string name;
    FindAttribute(NAME_ATTRIBUTE, name_folder_component->properties, name, FallbackMode::REQUIRE_ATTRIBUTE);
    return name;
}

std::string PathProxy::GetFolder() const
{
    const Component* name_folder_component = FindComponentFromHash(NAME_FOLDER_COMPONENT, m_components);
    if(!name_folder_component)
        return "Unknown";

    std::string folder;
    FindAttribute(FOLDER_ATTRIBUTE, name_folder_component->properties, folder, FallbackMode::REQUIRE_ATTRIBUTE);
    return folder;
}

void PathProxy::SetSelected(bool selected)
{
}

bool PathProxy::Intersects(const math::Vector& world_position) const
{
    const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(m_entity_id);
    const bool inside_bb = math::PointInsideQuad(world_position, world_bb);
    if(inside_bb)
    {
        const Component* path_component = FindComponentFromHash(PATH_COMPONENT, m_components);
        if(!path_component)
            return true;

        const Attribute* path_points_attribute = nullptr;
        const bool found_path = FindAttribute(PATH_POINTS_ATTRIBUTE, path_component->properties, path_points_attribute);
        if(!found_path)
            return true;

        const math::Matrix world_to_local = math::Inverse((const math::Matrix&)m_transform_system->GetTransform(m_entity_id));
        const math::Vector& local_position = math::Transform(world_to_local, world_position);

        const std::vector<math::Vector>& local_points = std::get<std::vector<math::Vector>>(path_points_attribute->value);

        float min_distance = math::INF;

        for(size_t index = 0; index < local_points.size() -1; ++index)
        {
            const math::PointOnLineResult result = math::ClosestPointOnLine(local_points[index], local_points[index+1], local_position);
            const float distance = math::DistanceBetween(result.point, local_position);
            min_distance = std::min(min_distance, distance);
        }

        const float threshold = m_editor->GetPickingDistance();
        return (min_distance < threshold);
    }

    return false;
}

bool PathProxy::Intersects(const math::Quad& world_bb) const
{
    const math::Quad& proxy_world_bb = m_transform_system->GetWorldBoundingBox(m_entity_id);
    const bool inside_bb = math::QuadOverlaps(proxy_world_bb, world_bb);
    return inside_bb;
}

std::vector<Grabber> PathProxy::GetGrabbers()
{
    std::vector<Grabber> grabbers;

    Component* path_component = FindComponentFromHash(PATH_COMPONENT, m_components);
    if(!path_component)
        return grabbers;

    Attribute* path_points_attribute = nullptr;
    const bool found_path = FindAttribute(PATH_POINTS_ATTRIBUTE, path_component->properties, path_points_attribute);
    if(!found_path)
        return grabbers;

    const math::Matrix& local_to_world = m_transform_system->GetTransform(m_entity_id);
    std::vector<math::Vector>& points = std::get<std::vector<math::Vector>>(path_points_attribute->value);
    grabbers.reserve(points.size());

    for(math::Vector& point : points)
    {
        Grabber grabber;
        grabber.position = math::Transform(local_to_world, static_cast<const math::Vector&>(point));
        grabber.callback = [path_component, local_to_world, &point, this](const math::Vector& new_position) {
            const math::Matrix& world_to_local = math::Inverse(local_to_world);
            point = math::Transform(world_to_local, new_position);

            m_entity_manager->SetComponentData(m_entity_id, path_component->hash, path_component->properties);
        };

        grabbers.push_back(grabber);
    }

    return grabbers;
}

std::vector<SnapPoint> PathProxy::GetSnappers() const
{
    return std::vector<SnapPoint>();
}

const std::vector<Component>& PathProxy::GetComponents() const
{
    return m_components;
}

std::vector<Component>& PathProxy::GetComponents()
{
    return m_components;
}

void PathProxy::ComponentChanged(Component& component, uint32_t attribute_hash)
{
    m_entity_manager->SetComponentData(m_entity_id, component.hash, component.properties);
    m_editor->UpdateGrabbers();
}

float PathProxy::GetRotation() const
{
    const math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);
    return math::GetZRotation(transform);
}

void PathProxy::SetRotation(float rotation)
{
    math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);
    const math::Vector position = math::GetPosition(transform);
    transform = math::CreateMatrixFromZRotation(rotation);
    math::Position(transform, position);
}

math::Vector PathProxy::GetPosition() const
{
    const math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);
    return math::GetPosition(transform);
}

void PathProxy::SetPosition(const math::Vector& position)
{
    math::Vector new_position = position;

    const bool snap_position = m_editor->SnapToGrid();
    if(snap_position)
    {
        const math::Vector grid_size = m_editor->GridSize();
        new_position.x = math::Align(position.x, grid_size.x);
        new_position.y = math::Align(position.y, grid_size.y);
    }

    math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);
    math::Position(transform, new_position);
}

math::Quad PathProxy::GetBoundingBox() const
{
    return m_transform_system->GetWorldBoundingBox(m_entity_id);
}

std::unique_ptr<editor::IObjectProxy> PathProxy::Clone() const
{
    return nullptr;
}

void PathProxy::Visit(IObjectVisitor& visitor)
{
    visitor.Accept(this);
}
