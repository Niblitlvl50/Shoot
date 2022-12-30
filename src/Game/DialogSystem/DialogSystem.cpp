
#include "DialogSystem.h"
#include "System/Hash.h"

using namespace game;

DialogSystem::DialogSystem(uint32_t n)
    : m_components(n)
{ }

DialogComponent* DialogSystem::AllocateComponent(uint32_t entity_id)
{
    return m_components.Set(entity_id, DialogComponent());
}

void DialogSystem::ReleaseComponent(uint32_t entity_id)
{
    m_components.Release(entity_id);
}

void DialogSystem::AddComponent(uint32_t entity_id, const std::string& message, float duration)
{
    DialogComponent* component = m_components.Get(entity_id);
    component->message = message;
    component->duration = duration;
}

const char* DialogSystem::Name() const
{
    return "dialogsystem";
}

void DialogSystem::Update(const mono::UpdateContext& update_context)
{

}
