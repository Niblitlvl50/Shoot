
#include "EntityProxy.h"
#include "Grabber.h"
#include "SnapPoint.h"
#include "SpriteEntity.h"
#include "UIContext.h"

#include "Math/Quad.h"
#include "Math/MathFunctions.h"

#include "ImGuiImpl/ImGuiImpl.h"

using namespace editor;

EntityProxy::EntityProxy(const std::shared_ptr<SpriteEntity>& entity)
    : m_entity(entity)
{ }

unsigned int EntityProxy::Id() const
{
    return m_entity->Id();
}

mono::IEntityPtr EntityProxy::Entity()
{
    return m_entity;
}

void EntityProxy::SetSelected(bool selected)
{
    m_entity->SetSelected(selected);
}

bool EntityProxy::Intersects(const math::Vector& position) const
{
    const math::Quad& bb = m_entity->BoundingBox();
    return math::PointInsideQuad(position, bb);
}

std::vector<Grabber> EntityProxy::GetGrabbers() const
{
    return std::vector<Grabber>();
}

std::vector<SnapPoint> EntityProxy::GetSnappers() const
{
    return std::vector<SnapPoint>();
}

void EntityProxy::UpdateUIContext(UIContext& context) const
{
    const std::string& name = m_entity->Name();
    const math::Vector& position = m_entity->Position();
    const float rotation = m_entity->Rotation();

    ImGui::Text("%s", name.c_str());
    
    ImGui::Value("X", position.x);
    ImGui::SameLine();
    ImGui::Value("Y", position.y);
    ImGui::Value("Rotation", rotation);
}
