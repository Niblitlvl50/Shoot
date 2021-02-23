
#include "ComponentProxy.h"
#include "SnapPoint.h"
#include "Grabber.h"

#include "EntitySystem/ObjectAttribute.h"
#include "Component.h"
#include "UI/UIProperties.h"
#include "IObjectVisitor.h"
#include "Math/MathFunctions.h"
#include "Math/Vector.h"

#include "EntitySystem/IEntityManager.h"
#include "Entity/EntityProperties.h"
#include "TransformSystem/TransformSystem.h"

#include "Editor.h"

using namespace editor;

ComponentProxy::ComponentProxy(
    uint32_t entity_id,
    const std::string& name,
    const std::string& folder,
    const std::vector<Component>& components,
    mono::IEntityManager* entity_manager,
    mono::TransformSystem* transform_system,
    Editor* editor)
    : m_entity_id(entity_id)
    , m_name(name)
    , m_folder(folder)
    , m_entity_properties(0)
    , m_components(components)
    , m_entity_manager(entity_manager)
    , m_transform_system(transform_system)
    , m_editor(editor)
{
    shared::SortComponentsByPriority(m_components);
}

ComponentProxy::~ComponentProxy()
{
    m_entity_manager->ReleaseEntity(m_entity_id);
}

const char* ComponentProxy::Name() const
{
    return m_name.c_str();
}

uint32_t ComponentProxy::Id() const
{
    return m_entity_id;
}

void ComponentProxy::SetSelected(bool selected)
{

}

bool ComponentProxy::Intersects(const math::Vector& position) const
{
    const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(m_entity_id);
    return math::PointInsideQuad(position, world_bb);
}

std::vector<Grabber> ComponentProxy::GetGrabbers()
{
    std::vector<Grabber> grabbers;

    Component* polygon_shape_component = FindComponentFromHash(POLYGON_SHAPE_COMPONENT, m_components);
    if(!polygon_shape_component)
        return grabbers;

    Attribute* polygon_attribute = nullptr;
    const bool found_polygon = FindAttribute(POLYGON_ATTRIBUTE, polygon_shape_component->properties, polygon_attribute);
    if(!found_polygon)
        return grabbers;

    const math::Matrix& local_to_world = m_transform_system->GetTransform(m_entity_id);
    std::vector<math::Vector>& points = std::get<std::vector<math::Vector>>(polygon_attribute->value);
    grabbers.reserve(points.size());

    for(math::Vector& point : points)
    {
        Grabber grabber;
        grabber.position = math::Transform(local_to_world, static_cast<const math::Vector&>(point));
        grabber.callback = [&local_to_world, &point](const math::Vector& new_position) {
            const math::Matrix& world_to_local = math::Inverse(local_to_world);
            point = math::Transform(world_to_local, new_position);
        };

        grabbers.push_back(grabber);
    }

    return grabbers;
}

std::vector<SnapPoint> ComponentProxy::GetSnappers() const
{
    return { };
}

void ComponentProxy::UpdateUIContext(UIContext& context)
{
    DrawStringProperty("Name", m_name);
    DrawStringProperty("Folder", m_folder);
    //DrawBitfieldProperty(m_entity_properties, all_entity_properties, EntityPropertyToString);
    
    const DrawComponentsResult result = DrawComponents(context, m_components);
    if(result.component_index == std::numeric_limits<uint32_t>::max())
        return;

    const Component& modified_component = m_components[result.component_index];
    m_entity_manager->SetComponentData(m_entity_id, modified_component.hash, modified_component.properties);

    // Special case for setting a proper name...
    if(modified_component.hash == SPRITE_COMPONENT && m_name == "unnamed")
    {
        std::string sprite_name;
        const bool found_sprite_name = 
            FindAttribute(SPRITE_ATTRIBUTE, modified_component.properties, sprite_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(found_sprite_name)
        {
            m_name = sprite_name;
            const size_t dot_pos = m_name.find_last_of('.');
            m_name.erase(dot_pos);
        }
    }

    if(result.attribute_hash == POLYGON_ATTRIBUTE)
        m_editor->UpdateGrabbers();
}

void ComponentProxy::SetFolder(const std::string& folder)
{
    m_folder = folder;
}

std::string ComponentProxy::GetFolder() const
{
    return m_folder;
}

const std::vector<Component>& ComponentProxy::GetComponents() const
{
    return m_components;
}

std::vector<Component>& ComponentProxy::GetComponents()
{
    return m_components;
}

uint32_t ComponentProxy::GetEntityProperties() const
{
    return m_entity_properties;
}

void ComponentProxy::SetEntityProperties(uint32_t properties)
{
    m_entity_properties = properties;
}

math::Vector ComponentProxy::GetPosition() const
{
    const math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);
    return math::GetPosition(transform);
}

void ComponentProxy::SetPosition(const math::Vector& position)
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

float ComponentProxy::GetRotation() const
{
    const math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);
    return math::GetZRotation(transform);
}

void ComponentProxy::SetRotation(float rotation)
{
    math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);
    const math::Vector position = math::GetPosition(transform);
    transform = math::CreateMatrixFromZRotation(rotation);
    math::Position(transform, position);
}

math::Quad ComponentProxy::GetBoundingBox() const
{
    return m_transform_system->GetWorldBoundingBox(m_entity_id);
}

std::unique_ptr<editor::IObjectProxy> ComponentProxy::Clone() const
{
    const mono::Entity new_entity = m_entity_manager->CreateEntity("", {});
    auto proxy = std::make_unique<ComponentProxy>(
        new_entity.id, Name(), m_folder, m_components, m_entity_manager, m_transform_system, m_editor);

    for(const Component& component : m_components)
    {
        m_entity_manager->AddComponent(new_entity.id, component.hash);
        m_entity_manager->SetComponentData(new_entity.id, component.hash, component.properties);
    }

    return proxy;
}

void ComponentProxy::Visit(IObjectVisitor& visitor)
{
    visitor.Accept(this);
}
