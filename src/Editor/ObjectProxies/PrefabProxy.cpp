
#include "PrefabProxy.h"
#include "IObjectVisitor.h"
#include "Objects/Prefab.h"
#include "Grabber.h"
#include "SnapPoint.h"
#include "UI/UIContext.h"

#include "Math/Quad.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Math/MathFunctions.h"

#include "ImGuiImpl/ImGuiImpl.h"

#include "ObjectAttribute.h"
#include "Component.h"

using namespace editor;

PrefabProxy::PrefabProxy(const std::shared_ptr<Prefab>& prefab)
    : m_prefab(prefab)
{ }

PrefabProxy::~PrefabProxy()
{ }

const char* PrefabProxy::Name() const
{
    return m_prefab->Name().c_str();
}

unsigned int PrefabProxy::Id() const
{
    return m_prefab->Id();
}

mono::IEntityPtr PrefabProxy::Entity()
{
    return m_prefab;
}

void PrefabProxy::SetSelected(bool selected)
{
    m_prefab->SetSelected(selected);
}

bool PrefabProxy::Intersects(const math::Vector& world_position) const
{
    const math::Quad& bb = m_prefab->BoundingBox();
    return math::PointInsideQuad(world_position, bb);
}

std::vector<Grabber> PrefabProxy::GetGrabbers() const
{
    return std::vector<Grabber>();
}

std::vector<SnapPoint> PrefabProxy::GetSnappers() const
{
    const math::Matrix& matrix = m_prefab->Transformation();
    const float rotation = m_prefab->Rotation();

    const auto func = [&matrix, rotation](SnapPoint& point) {
        math::Transform(matrix, point.position);
        point.normal -= rotation;
    };

    std::vector<SnapPoint> snappers = m_prefab->SnapPoints();
    std::for_each(snappers.begin(), snappers.end(), func);

    return snappers;
}

void PrefabProxy::UpdateUIContext(UIContext& context)
{
    const std::string& name = m_prefab->Name();
    const math::Vector& position = m_prefab->Position();
    
    float rotation = m_prefab->Rotation();

    ImGui::Text("%s", name.c_str());
    
    ImGui::Value("X", position.x);
    ImGui::SameLine();
    ImGui::Value("Y", position.y);    
    ImGui::InputFloat("Rotation", &rotation);

    m_prefab->SetRotation(rotation);
}

const std::vector<Component>& PrefabProxy::GetComponents() const
{
    return m_components;
}

std::vector<Component>& PrefabProxy::GetComponents()
{
    return m_components;
}

std::unique_ptr<editor::IObjectProxy> PrefabProxy::Clone() const
{
    return nullptr;
}

void PrefabProxy::Visit(IObjectVisitor& visitor)
{
    visitor.Accept(this);
}
