
#include "PathProxy.h"
#include "IObjectVisitor.h"
#include "Grabber.h"
#include "SnapPoint.h"
#include "UI/UIContext.h"
#include "Objects/Path.h"
#include "Editor.h"
#include "Math/Matrix.h"
#include "Math/Quad.h"
#include "Math/MathFunctions.h"

#include "ImGuiImpl/ImGuiImpl.h"
#include "ObjectAttribute.h"
#include "Component.h"

using namespace editor;

PathProxy::PathProxy(const std::shared_ptr<PathEntity>& path, Editor* editor)
    : m_path(path),
      m_editor(editor)
{ }

PathProxy::~PathProxy()
{ }

const char* PathProxy::Name() const
{
    return m_path->GetName().c_str();
}

unsigned int PathProxy::Id() const
{
    return m_path->Id();
}

mono::IEntityPtr PathProxy::Entity()
{
    return m_path;
}

void PathProxy::SetSelected(bool selected)
{
    m_path->SetSelected(selected);
}

bool PathProxy::Intersects(const math::Vector& world_position) const
{
    const math::Quad& bb = m_path->BoundingBox();
    const bool inside_bb = math::PointInsideQuad(world_position, bb);
    if(inside_bb)
    {
        math::Matrix world_to_local = m_path->Transformation();
        math::Inverse(world_to_local);
        const math::Vector& local_position = math::Transform(world_to_local, world_position);

        float min_distance = math::INF;

        const auto& local_points = m_path->GetPoints();
        for(size_t index = 0; index < local_points.size() -1; ++index)
        {
            const math::Vector& line_point = math::ClosestPointOnLine(local_points[index], local_points[index+1], local_position);
            const float length = math::Length(line_point - local_position);
            min_distance = std::min(min_distance, length);
        }

        const float threshold = m_editor->GetPickingDistance();
        return (min_distance < threshold);
    }

    return false;
}

std::vector<Grabber> PathProxy::GetGrabbers() const
{
    using namespace std::placeholders;

    const math::Matrix& local_to_world = m_path->Transformation();
    const auto& local_points = m_path->GetPoints();

    std::vector<Grabber> grabbers;
    grabbers.reserve(local_points.size());

    for(size_t index = 0; index < local_points.size(); ++index)
    {
        Grabber grab;
        grab.position = math::Transform(local_to_world, local_points[index]);
        grab.callback = std::bind(&PathEntity::SetVertex, m_path, _1, index);
        grabbers.push_back(grab);
    }

    return grabbers;
}

std::vector<SnapPoint> PathProxy::GetSnappers() const
{
    return std::vector<SnapPoint>();
}

void PathProxy::UpdateUIContext(UIContext& context)
{
    const std::string& name = m_path->GetName();
    const math::Vector& position = m_path->Position();

    char buffer[100] = { 0 };
    snprintf(buffer, 100, "%s", name.c_str());

    if(ImGui::InputText("", buffer, 100))
        m_path->SetName(buffer);
    
    ImGui::Value("X", position.x);
    ImGui::SameLine();
    ImGui::Value("Y", position.y);
    ImGui::Value("Rotation", m_path->Rotation());
}

const std::vector<Component>& PathProxy::GetComponents() const
{
    return m_components;
}

std::vector<Component>& PathProxy::GetComponents()
{
    return m_components;
}

float PathProxy::GetRotation() const
{
    return m_path->Rotation();
}

void PathProxy::SetRotation(float rotation)
{
    m_path->SetRotation(rotation);
}

math::Vector PathProxy::GetPosition() const
{
    return m_path->Position();
}

void PathProxy::SetPosition(const math::Vector& position)
{
    m_path->SetPosition(position);
}

std::unique_ptr<editor::IObjectProxy> PathProxy::Clone() const
{
    return nullptr;
}

void PathProxy::Visit(IObjectVisitor& visitor)
{
    visitor.Accept(this);
}
