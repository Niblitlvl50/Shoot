
#include "DialogSystem.h"
#include "System/Hash.h"

using namespace game;

DialogSystem::DialogSystem(uint32_t n)
{
    m_components.resize(n);
    m_active.resize(n, false);
}

DialogComponent* DialogSystem::AllocateComponent(uint32_t entity_id)
{
    m_active[entity_id] = true;
    return &m_components[entity_id];
}

void DialogSystem::ReleaseComponent(uint32_t entity_id)
{
    m_active[entity_id] = false;
}

void DialogSystem::AddComponent(uint32_t entity_id, const std::string& message, float duration)
{
    DialogComponent& component = m_components[entity_id];
    component.message = message;
    component.duration = duration;
}

uint32_t DialogSystem::Id() const
{
    return hash::Hash(Name());
}

const char* DialogSystem::Name() const
{
    return "dialogsystem";
}

void DialogSystem::Update(const mono::UpdateContext& update_context)
{

}
