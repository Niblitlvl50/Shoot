
#include "RegionSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/IShape.h"
#include "CollisionConfiguration.h"

#include "System/Hash.h"

using namespace game;

namespace
{
    class RegionCollisionHandler : public mono::ICollisionHandler
    {
    public:

        RegionCollisionHandler(uint32_t entity_id, game::RegionSystem* region_system)
            : m_entity_id(entity_id)
            , m_region_system(region_system)
        { }

        mono::CollisionResolve OnCollideWith(
            mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t categories) override
        {
            m_region_system->ActivateRegion(m_entity_id);
            return mono::CollisionResolve::NORMAL;
        }

        void OnSeparateFrom(mono::IBody* body) override
        { }

        const uint32_t m_entity_id;
        game::RegionSystem* m_region_system;
    };
}

RegionSystem::RegionSystem(mono::PhysicsSystem* physics_system)
    : m_physics_system(physics_system)
{
    Reset();
}

const char* RegionSystem::Name() const
{
    return "regionsystem";
}

void RegionSystem::Reset()
{
    m_active_region = { 0, "", "" };
    m_activated_regions.clear();
}

void RegionSystem::Update(const mono::UpdateContext& update_context)
{

}

void RegionSystem::AllocateRegion(uint32_t entity_id)
{
    m_regions[entity_id];
}

void RegionSystem::ReleaseRegion(uint32_t entity_id)
{
    RegionComponent& component = m_regions[entity_id];

    if(component.collision_handler)
    {
        mono::IBody* body = m_physics_system->GetBody(entity_id);
        if(body)
            body->RemoveCollisionHandler(component.collision_handler.get());

        component.collision_handler = nullptr;
    }

    m_regions.erase(entity_id);
}

void RegionSystem::UpdateRegion(uint32_t entity_id, const std::string& text, const std::string& sub_text)
{
    RegionComponent& component = m_regions[entity_id];
    component.text = text;
    component.sub_text = sub_text;

    mono::IBody* body = m_physics_system->GetBody(entity_id);
    if(body)
    {
        if(component.collision_handler)
            body->RemoveCollisionHandler(component.collision_handler.get());

        component.collision_handler = std::make_unique<RegionCollisionHandler>(entity_id, this);
        body->AddCollisionHandler(component.collision_handler.get());

        std::vector<mono::IShape*> shapes = m_physics_system->GetShapesAttachedToBody(entity_id);
        for(mono::IShape* shape : shapes)
        {
            if(shape->IsSensor())
                shape->SetCollisionMask(CollisionCategory::PLAYER);
        }
    }
}

void RegionSystem::ActivateRegion(uint32_t entity_id)
{
    const auto it = m_regions.find(entity_id);
    if(it != m_regions.end() && (m_activated_regions.count(entity_id) == 0))
    {
        m_active_region.text = it->second.text;
        m_active_region.sub_text = it->second.sub_text;
        m_active_region.entity_id = entity_id;

        m_activated_regions.insert(entity_id);
    }
}

const RegionDescription& RegionSystem::GetActivatedRegion() const
{
    return m_active_region;
}
