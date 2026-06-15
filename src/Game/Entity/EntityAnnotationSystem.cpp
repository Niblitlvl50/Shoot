
#include "EntityAnnotationSystem.h"

#include "Math/MathFunctions.h"
#include "Math/Quad.h"
#include "TransformSystem/TransformSystem.h"
#include "EntitySystem/IEntityManager.h"
#include "EntitySystem/Entity.h"

using namespace game;

EntityAnnotationSystem::EntityAnnotationSystem(
    mono::TransformSystem* transform_system, mono::IEntityManager* entity_manager)
    : m_transform_system(transform_system)
    , m_entity_manager(entity_manager)
{ }

uint32_t EntityAnnotationSystem::AddAnnotation(
    uint32_t entity_id, const std::string& entity_file, AnnotationCorner corner)
{
    const math::Quad& bb = m_transform_system->GetBoundingBox(entity_id);

    math::Vector corner_position;
    switch(corner)
    {
    case AnnotationCorner::TopRight:
        corner_position = math::TopRight(bb) / 2.0f;
        break;
    case AnnotationCorner::TopLeft:
        corner_position = math::TopLeft(bb) / 2.0f;
        break;
    case AnnotationCorner::BottomRight:
        corner_position = math::BottomRight(bb) / 2.0f;
        break;
    case AnnotationCorner::BottomLeft:
        corner_position = math::BottomLeft(bb) / 2.0f;
        break;
    }

    const mono::Entity spawned = m_entity_manager->SpawnEntity(entity_file.c_str());
    m_transform_system->SetTransform(spawned.id, math::CreateMatrixWithPosition(corner_position), mono::TransformState::CLIENT);
    m_transform_system->ChildTransform(spawned.id, entity_id);
    m_entity_manager->SetLifetimeDependency(entity_id, spawned.id);

    return spawned.id;
}

void EntityAnnotationSystem::RemoveAnnotation(uint32_t annotation_entity_id)
{
    m_entity_manager->ReleaseEntity(annotation_entity_id);
}

const char* EntityAnnotationSystem::Name() const
{
    return "entityannotationsystem";
}

void EntityAnnotationSystem::Update(const mono::UpdateContext& update_context)
{ }
