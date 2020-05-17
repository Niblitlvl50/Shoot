
#include "ComponentProxy.h"
#include "SnapPoint.h"
#include "Grabber.h"

#include "ObjectAttribute.h"
#include "Component.h"
#include "UI/UIProperties.h"
#include "IObjectVisitor.h"
#include "Math/MathFunctions.h"

#include "Entity/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"

using namespace editor;


ComponentProxy::ComponentProxy(
    uint32_t entity_id,
    const std::string& name,
    const std::string& folder,
    const std::vector<Component>& components,
    IEntityManager* entity_manager,
    mono::TransformSystem* transform_system)
    : m_entity_id(entity_id)
    , m_name(name)
    , m_folder(folder)
    , m_entity_properties(0)
    , m_components(components)
    , m_entity_manager(entity_manager)
    , m_transform_system(transform_system)
{
    for(const Component& component : m_components)
    {
        entity_manager->AddComponent(entity_id, component.hash);
        entity_manager->SetComponentData(entity_id, component.hash, component.properties);
    }

/*
    const auto body_before_shapes = [](const Component& left, const Component& right) {

        const auto is_shape_component = [](uint32_t hash) {
            return 
                hash == CIRCLE_SHAPE_COMPONENT ||
                hash == BOX_SHAPE_COMPONENT ||
                hash == SEGMENT_SHAPE_COMPONENT;
        };

        if(left.hash == TRANSFORM_COMPONENT)
            return true;
        else if(right.hash == TRANSFORM_COMPONENT)
            return false;

        if(left.hash == PHYSICS_COMPONENT && is_shape_component(right.hash))
        {
            return true;
        }
        else if(right.hash == PHYSICS_COMPONENT && is_shape_component(left.hash))
        {
            return false;
        }

        return false;
    };

    std::sort(m_components.begin(), m_components.end(), body_before_shapes);
    */
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
    //DrawEntityProperty(m_entity_properties);
    
    const int modified_index = DrawComponents(context, m_components);
    if(modified_index != -1)
    {
        Component& modified_component = m_components[modified_index];
        m_entity_manager->SetComponentData(m_entity_id, modified_component.hash, modified_component.properties);
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
    math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);
    math::Position(transform, position);
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
    return std::make_unique<ComponentProxy>(
        new_entity.id, Name(), m_folder, m_components, m_entity_manager, m_transform_system);
}

void ComponentProxy::Visit(IObjectVisitor& visitor)
{
    visitor.Accept(this);
}
