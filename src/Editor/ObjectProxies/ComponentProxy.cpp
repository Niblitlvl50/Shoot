
#include "ComponentProxy.h"
#include "SnapPoint.h"
#include "Grabber.h"

#include "EntitySystem/ObjectAttribute.h"
#include "Component.h"
#include "UI/UIProperties.h"
#include "IObjectVisitor.h"
#include "Math/MathFunctions.h"

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
{ }

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

mono::IEntity* ComponentProxy::Entity()
{
    return nullptr;
}

void ComponentProxy::SetSelected(bool selected)
{

}

bool ComponentProxy::Intersects(const math::Vector& position) const
{
    const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(m_entity_id);
    return math::PointInsideQuad(position, world_bb);
}

std::vector<Grabber> ComponentProxy::GetGrabbers() const
{
    return { };
}

std::vector<SnapPoint> ComponentProxy::GetSnappers() const
{
    return { };
}

void ComponentProxy::UpdateUIContext(UIContext& context)
{
    DrawName(m_name);
    DrawFolder(m_folder);
    //DrawBitfieldProperty(m_entity_properties, all_entity_properties, EntityPropertyToString);
    
    const int modified_index = DrawComponents(context, m_components);
    if(modified_index != -1)
    {
        Component& modified_component = m_components[modified_index];
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
    }
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
