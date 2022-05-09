
#include "StaggerBehaviour.h"
#include "Util/Random.h"
#include "IUpdatable.h"

using namespace game;

StaggerBehaviour::StaggerBehaviour()
    : StaggerBehaviour(0.0f, 0.0f)
{ }

StaggerBehaviour::StaggerBehaviour(float stagger_chance_fraction, float stagger_duration)
    : m_stagger_chance(stagger_chance_fraction)
    , m_stagger_duration(stagger_duration)
    , m_stagger_timer(0.0f)
    , m_staggering(false)
{ }

void StaggerBehaviour::Update(const mono::UpdateContext& update_context)
{
    if(!m_staggering)
        return;

    m_stagger_timer += update_context.delta_s;
    m_staggering = (m_stagger_timer < m_stagger_duration);
}

bool StaggerBehaviour::IsStaggering() const
{
    return m_staggering;
}

bool StaggerBehaviour::TestForStaggering()
{
    if(m_staggering)
        return true;

    m_staggering = mono::Chance(m_stagger_chance * 100.0f);
    if(m_staggering)
        m_stagger_timer = 0.0f;

    return m_staggering;
}

void StaggerBehaviour::SetChanceAndDuration(float chance_fraction, float duration_s)
{
    m_stagger_chance = chance_fraction;
    m_stagger_duration = duration_s;
}
