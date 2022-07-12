
#include "ComponentProxy.h"
#include "SnapPoint.h"
#include "Grabber.h"

#include "EntitySystem/ObjectAttribute.h"
#include "Entity/Component.h"
#include "UI/UIProperties.h"
#include "IObjectVisitor.h"
#include "Math/MathFunctions.h"
#include "Math/Vector.h"

#include "EntitySystem/IEntityManager.h"
#include "Entity/EntityProperties.h"
#include "TransformSystem/TransformSystem.h"

#include "Editor.h"

#include "imgui/imgui.h"

#include <cassert>
#include <algorithm>

using namespace editor;

ComponentProxy::ComponentProxy(
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
{
    component::SortComponentsByPriority(m_components);
}

ComponentProxy::~ComponentProxy()
{
    m_entity_manager->ReleaseEntity(m_entity_id);
}

uint32_t ComponentProxy::Uuid() const
{
    return m_entity_manager->GetEntityUuid(m_entity_id);
}

uint32_t ComponentProxy::Id() const
{
    return m_entity_id;
}

std::string ComponentProxy::Name() const
{
    const Component* name_folder_component = FindComponentFromHash(NAME_FOLDER_COMPONENT, m_components);
    if(!name_folder_component)
        return "Unknown";

    std::string name;
    FindAttribute(NAME_ATTRIBUTE, name_folder_component->properties, name, FallbackMode::REQUIRE_ATTRIBUTE);
    return name;
}

void ComponentProxy::SetName(const std::string& name)
{
    Component* name_folder_component = FindComponentFromHash(NAME_FOLDER_COMPONENT, m_components);
    assert(name_folder_component != nullptr);
    SetAttribute(NAME_ATTRIBUTE, name_folder_component->properties, name);
}

void ComponentProxy::SetFolder(const std::string& folder)
{
    Component* name_folder_component = FindComponentFromHash(NAME_FOLDER_COMPONENT, m_components);
    assert(name_folder_component != nullptr);
    SetAttribute(FOLDER_ATTRIBUTE, name_folder_component->properties, folder);
}

std::string ComponentProxy::GetFolder() const
{
    const Component* name_folder_component = FindComponentFromHash(NAME_FOLDER_COMPONENT, m_components);
    if(!name_folder_component)
        return "Unknown";

    std::string folder;
    FindAttribute(FOLDER_ATTRIBUTE, name_folder_component->properties, folder, FallbackMode::REQUIRE_ATTRIBUTE);
    return folder;
}

bool ComponentProxy::Intersects(const math::Vector& position) const
{
    const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(m_entity_id);
    return math::PointInsideQuad(position, world_bb);
}

bool ComponentProxy::Intersects(const math::Quad& world_bb) const
{
    const math::Quad& proxy_world_bb = m_transform_system->GetWorldBoundingBox(m_entity_id);
    return math::QuadOverlaps(world_bb, proxy_world_bb);
}

std::vector<Grabber> ComponentProxy::GetGrabbers()
{
    const uint32_t components_with_grabbers[] = {
        POLYGON_ATTRIBUTE,
        PATH_POINTS_ATTRIBUTE
    };

    struct ComponentAndAttribute
    {
        Component* component;
        Attribute* attribute;
    };

    std::vector<ComponentAndAttribute> grabber_attributes;

    for(Component& component : m_components)
    {
        for(Attribute& attribute : component.properties)
        {
            const bool valid_attribute = std::any_of(
                std::begin(components_with_grabbers),
                std::end(components_with_grabbers),
                [&attribute](uint32_t id) { return attribute.id == id; }
            );

            if(valid_attribute)
                grabber_attributes.push_back({&component, &attribute});
        }
    }

    const math::Matrix& local_to_world = m_transform_system->GetTransform(m_entity_id);

    std::vector<Grabber> grabbers;

    for(ComponentAndAttribute& comp_attr : grabber_attributes)
    {
        const Component* local_component = comp_attr.component;
        std::vector<math::Vector>& points = std::get<std::vector<math::Vector>>(comp_attr.attribute->value);
        for(math::Vector& point : points)
        {
            Grabber grabber;
            grabber.position = math::Transformed(local_to_world, static_cast<const math::Vector&>(point));
            grabber.callback = [this, local_component, &local_to_world, &point](const math::Vector& new_position) {
                const math::Matrix& world_to_local = math::Inverse(local_to_world);
                point = math::Transformed(world_to_local, new_position);
                m_entity_manager->SetComponentData(m_entity_id, local_component->hash, local_component->properties);
            };

            grabbers.push_back(grabber);
        }
    }

    return grabbers;
}

std::vector<SnapPoint> ComponentProxy::GetSnappers() const
{
    return { };
}

const std::vector<Component>& ComponentProxy::GetComponents() const
{
    return m_components;
}

std::vector<Component>& ComponentProxy::GetComponents()
{
    return m_components;
}

void ComponentProxy::ComponentChanged(Component& component, uint32_t attribute_hash)
{
    m_entity_manager->SetComponentData(m_entity_id, component.hash, component.properties);

    const std::string name = Name();

    if(component.hash == NAME_FOLDER_COMPONENT)
        m_entity_manager->SetEntityEnabled(m_entity_id, name.c_str());

    const bool unnamed_or_empty = (name == "unnamed" || name.empty());

    // Special case for setting a proper name...
    if(component.hash == SPRITE_COMPONENT && unnamed_or_empty)
    {
        std::string sprite_name;
        const bool found_sprite_name = 
            FindAttribute(SPRITE_ATTRIBUTE, component.properties, sprite_name, FallbackMode::REQUIRE_ATTRIBUTE);
        if(found_sprite_name)
        {
            const size_t dot_pos = sprite_name.find_last_of('.');
            sprite_name.erase(dot_pos);
            SetName(sprite_name);
        }
    }

    if(attribute_hash == POLYGON_ATTRIBUTE || attribute_hash == PATH_POINTS_ATTRIBUTE)
        m_editor->UpdateGrabbers();
}

uint32_t ComponentProxy::GetEntityProperties() const
{
    return m_entity_manager->GetEntityProperties(m_entity_id);
}

void ComponentProxy::SetEntityProperties(uint32_t properties)
{
    m_entity_manager->SetEntityProperties(m_entity_id, properties);
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

    Component* transform_component = FindComponentFromHash(TRANSFORM_COMPONENT, m_components);
    if(transform_component)
        SetAttribute(POSITION_ATTRIBUTE, transform_component->properties, new_position);
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

    Component* transform_component = FindComponentFromHash(TRANSFORM_COMPONENT, m_components);
    if(transform_component)
        SetAttribute(ROTATION_ATTRIBUTE, transform_component->properties, rotation);
}

math::Quad ComponentProxy::GetBoundingBox() const
{
    return m_transform_system->GetWorldBoundingBox(m_entity_id);
}

std::unique_ptr<editor::IObjectProxy> ComponentProxy::Clone() const
{
    const mono::Entity new_entity = m_entity_manager->CreateEntity("", {});
    auto proxy = std::make_unique<ComponentProxy>(
        new_entity.id, m_components, m_entity_manager, m_transform_system, m_editor);

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
