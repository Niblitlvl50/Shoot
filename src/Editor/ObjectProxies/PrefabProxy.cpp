
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

using namespace editor;

PrefabProxy::PrefabProxy(const std::shared_ptr<Prefab>& prefab)
    : m_prefab(prefab)
{ }

const char* PrefabProxy::Name() const
{
    return "prefabobject";
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

bool PrefabProxy::Intersects(const math::Vector& position) const
{
    const math::Quad& bb = m_prefab->BoundingBox();
    return math::PointInsideQuad(position, bb);
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
    const float rotation = m_prefab->Rotation();

    ImGui::Text("%s", name.c_str());
    
    ImGui::Value("X", position.x);
    ImGui::SameLine();
    ImGui::Value("Y", position.y);
    ImGui::Value("Rotation", rotation);
}

std::vector<Attribute> PrefabProxy::GetAttributes() const
{
    return std::vector<Attribute>();
}

void PrefabProxy::SetAttributes(const std::vector<Attribute>& attributes)
{ }

void PrefabProxy::Visit(IObjectVisitor& visitor)
{
    visitor.Accept(this);
}
