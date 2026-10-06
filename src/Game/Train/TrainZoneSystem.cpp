
#include "TrainZoneSystem.h"
#include "TrainCarSystem.h"

#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/IBody.h"
#include "Physics/IShape.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Math/MathFunctions.h"
#include "System/Hash.h"

#include <limits>

namespace tweak_values
{
    // A car slower than this counts as stopped in a zone.
    constexpr float stopped_speed_threshold = 0.5f;
}

using namespace game;

TrainZoneSystem::TrainZoneSystem(
    mono::TransformSystem* transform_system,
    mono::PhysicsSystem* physics_system,
    mono::IEntityManager* entity_manager,
    mono::TriggerSystem* trigger_system,
    TrainCarSystem* train_car_system)
    : m_transform_system(transform_system)
    , m_physics_system(physics_system)
    , m_trigger_system(trigger_system)
    , m_entity_manager(entity_manager)
    , m_train_car_system(train_car_system)
{ }

void TrainZoneSystem::AllocateCargo(uint32_t entity_id)
{
    m_cargo[entity_id] = CargoComponent();
}

void TrainZoneSystem::ReleaseCargo(uint32_t entity_id)
{
    m_cargo.erase(entity_id);
}

void TrainZoneSystem::SetCargoData(uint32_t entity_id, int value, const std::string& destination)
{
    const auto it = m_cargo.find(entity_id);
    if(it == m_cargo.end())
        return;

    it->second.value = value;
    it->second.destination = destination;
}

void TrainZoneSystem::AllocateLoadingZone(uint32_t entity_id)
{
    m_loading_zones[entity_id] = { math::ZeroVec, hash::NO_HASH, 0.0f, std::string() };
}

void TrainZoneSystem::ReleaseLoadingZone(uint32_t entity_id)
{
    m_loading_zones.erase(entity_id);
}

void TrainZoneSystem::SetLoadingZoneData(uint32_t entity_id, const math::Vector& size, float transfer_duration_s, uint32_t trigger_hash)
{
    const auto it = m_loading_zones.find(entity_id);
    if(it == m_loading_zones.end())
        return;

    it->second.size = size;
    it->second.transfer_duration_s = transfer_duration_s;
    it->second.trigger_hash = trigger_hash;
}

void TrainZoneSystem::AllocateDropOffZone(uint32_t entity_id)
{
    m_drop_off_zones[entity_id] = { math::ZeroVec, hash::NO_HASH, 0.0f, std::string() };
}

void TrainZoneSystem::ReleaseDropOffZone(uint32_t entity_id)
{
    m_drop_off_zones.erase(entity_id);
}

void TrainZoneSystem::SetDropOffZoneData(
    uint32_t entity_id, const math::Vector& size, float transfer_duration_s, const std::string& destination, uint32_t trigger_hash)
{
    const auto it = m_drop_off_zones.find(entity_id);
    if(it == m_drop_off_zones.end())
        return;

    it->second.size = size;
    it->second.transfer_duration_s = transfer_duration_s;
    it->second.destination = destination;
    it->second.trigger_hash = trigger_hash;
}

const char* TrainZoneSystem::Name() const
{
    return "trainzonesystem";
}

void TrainZoneSystem::Update(const mono::UpdateContext& update_context)
{
    if(m_loading_zones.empty() && m_drop_off_zones.empty())
        return;

    for(auto& [timer_key, timer] : m_transfer_timers)
        timer.ticked = false;

    std::vector<uint32_t> stopped_cars;

    const std::vector<uint32_t> car_ids = m_train_car_system->GetCarIds();
    for(uint32_t car_id : car_ids)
    {
        if(m_train_car_system->IsLocomotive(car_id))
            continue;

        const mono::IBody* body = m_physics_system->GetBody(car_id);
        const float speed = math::Length(body->GetVelocity());
        if(speed < tweak_values::stopped_speed_threshold)
            stopped_cars.push_back(car_id);
    }

    for(const auto& [zone_entity_id, zone] : m_loading_zones)
        UpdateLoadingZone(zone_entity_id, zone, stopped_cars, update_context.delta_s);

    for(const auto& [zone_entity_id, zone] : m_drop_off_zones)
        UpdateDropOffZone(zone_entity_id, zone, stopped_cars, update_context.delta_s);

    // Timers that weren't ticked this frame start over next time.
    for(auto it = m_transfer_timers.begin(); it != m_transfer_timers.end();)
    {
        if(it->second.ticked)
            ++it;
        else
            it = m_transfer_timers.erase(it);
    }
}

void TrainZoneSystem::UpdateLoadingZone(
    uint32_t zone_entity_id, const TrainZoneComponent& zone, const std::vector<uint32_t>& stopped_cars, float delta_s)
{
    for(uint32_t car_id : stopped_cars)
    {
        if(!IsInsideZone(zone_entity_id, zone, car_id))
            continue;

        std::vector<uint32_t> cargo_to_load;

        for(const auto& [cargo_entity_id, cargo] : m_cargo)
        {
            if(cargo.delivered || !m_train_car_system->CanLoad(car_id, cargo.value))
                continue;

            if(m_train_car_system->FindCarCarrying(cargo_entity_id) != mono::INVALID_ID)
                continue;

            if(IsInsideZone(zone_entity_id, zone, cargo_entity_id))
                cargo_to_load.push_back(cargo_entity_id);
        }

        if(cargo_to_load.empty())
            continue;

        int n_transfers = TickTransferTimer(car_id, zone_entity_id, zone, delta_s);

        for(uint32_t cargo_entity_id : cargo_to_load)
        {
            if(n_transfers <= 0)
                break;

            CargoComponent& cargo = m_cargo[cargo_entity_id];
            const bool loaded = m_train_car_system->Load(car_id, cargo_entity_id, cargo.value);
            if(!loaded)
                continue;

            HideCargo(cargo_entity_id, cargo);
            --n_transfers;

            if(zone.trigger_hash != hash::NO_HASH)
                m_trigger_system->EmitTrigger(zone.trigger_hash);
        }
    }
}

void TrainZoneSystem::UpdateDropOffZone(
    uint32_t zone_entity_id, const TrainZoneComponent& zone, const std::vector<uint32_t>& stopped_cars, float delta_s)
{
    const math::Vector drop_off_position = m_transform_system->GetWorldPosition(zone_entity_id);

    for(uint32_t car_id : stopped_cars)
    {
        if(!IsInsideZone(zone_entity_id, zone, car_id))
            continue;

        std::vector<uint32_t> cargo_to_drop_off;

        for(const TrainCargo& train_cargo : m_train_car_system->GetCargo(car_id))
        {
            const auto cargo_it = m_cargo.find(train_cargo.entity_id);
            if(cargo_it != m_cargo.end() && cargo_it->second.destination == zone.destination)
                cargo_to_drop_off.push_back(train_cargo.entity_id);
        }

        if(cargo_to_drop_off.empty())
            continue;

        int n_transfers = TickTransferTimer(car_id, zone_entity_id, zone, delta_s);

        for(uint32_t cargo_entity_id : cargo_to_drop_off)
        {
            if(n_transfers <= 0)
                break;

            m_train_car_system->Unload(car_id, cargo_entity_id);

            CargoComponent& cargo = m_cargo[cargo_entity_id];
            cargo.delivered = true;
            ShowCargo(cargo_entity_id, cargo, drop_off_position);
            --n_transfers;

            if(zone.trigger_hash != hash::NO_HASH)
                m_trigger_system->EmitTrigger(zone.trigger_hash);
        }
    }
}

int TrainZoneSystem::TickTransferTimer(uint32_t car_id, uint32_t zone_entity_id, const TrainZoneComponent& zone, float delta_s)
{
    if(zone.transfer_duration_s <= 0.0f)
        return std::numeric_limits<int>::max();

    const uint64_t timer_key = (static_cast<uint64_t>(car_id) << 32) | zone_entity_id;
    TransferTimer& timer = m_transfer_timers[timer_key];

    timer.ticked = true;
    timer.elapsed_s += delta_s;

    if(timer.elapsed_s < zone.transfer_duration_s)
        return 0;

    timer.elapsed_s = 0.0f;
    return 1;
}

bool TrainZoneSystem::IsInsideZone(uint32_t zone_entity_id, const TrainZoneComponent& zone, uint32_t entity_id) const
{
    const math::Vector half_size = zone.size / 2.0f;
    const math::Matrix& zone_transform = m_transform_system->GetWorld(zone_entity_id);
    const math::Quad zone_bounds = math::Transformed(zone_transform, math::Quad(-half_size, half_size));

    const math::Vector position = m_transform_system->GetWorldPosition(entity_id);
    return math::PointInsideQuad(position, zone_bounds);
}

void TrainZoneSystem::HideCargo(uint32_t cargo_entity_id, CargoComponent& cargo)
{
    m_entity_manager->SetEntityEnabled(cargo_entity_id, false);

    cargo.saved_collision_masks.clear();

    if(m_physics_system->IsAllocated(cargo_entity_id))
    {
        const std::vector<mono::IShape*> shapes = m_physics_system->GetShapesAttachedToBody(cargo_entity_id);
        for(mono::IShape* shape : shapes)
        {
            cargo.saved_collision_masks.push_back(shape->GetCollisionMask());
            shape->SetCollisionMask(0);
        }

        m_physics_system->GetBody(cargo_entity_id)->SetVelocity(math::ZeroVec);
    }
}

void TrainZoneSystem::ShowCargo(uint32_t cargo_entity_id, CargoComponent& cargo, const math::Vector& position)
{
    m_transform_system->SetTransform(cargo_entity_id, math::CreateMatrixWithPosition(position));

    if(m_physics_system->IsAllocated(cargo_entity_id))
    {
        mono::IBody* body = m_physics_system->GetBody(cargo_entity_id);
        body->SetPosition(position);
        body->SetVelocity(math::ZeroVec);

        const std::vector<mono::IShape*> shapes = m_physics_system->GetShapesAttachedToBody(cargo_entity_id);
        for(size_t index = 0; index < shapes.size() && index < cargo.saved_collision_masks.size(); ++index)
            shapes[index]->SetCollisionMask(cargo.saved_collision_masks[index]);
    }

    cargo.saved_collision_masks.clear();
    m_entity_manager->SetEntityEnabled(cargo_entity_id, true);
}
