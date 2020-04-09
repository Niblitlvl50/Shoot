
#include "WaveDrawer.h"
#include "Events/WaveEvent.h"
#include "FontIds.h"

#include "EventHandler/EventHandler.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
#include <functional>

using namespace game;

WaveDrawer::WaveDrawer(mono::EventHandler& event_handler)
    : m_event_handler(event_handler)
{
    using namespace std::placeholders;
    const std::function<mono::EventResult (const WaveStartedEvent&)>& wave_started_func = std::bind(&WaveDrawer::WaveStarted, this, _1);
    const std::function<mono::EventResult (const HordeCompletedEvent&)>& horde_completed_func = std::bind(&WaveDrawer::HordeCompleted, this, _1);

    m_wave_started_token = m_event_handler.AddListener(wave_started_func);
    m_horde_completed_token = m_event_handler.AddListener(horde_completed_func);
}

WaveDrawer::~WaveDrawer()
{
    m_event_handler.RemoveListener(m_wave_started_token);
    m_event_handler.RemoveListener(m_horde_completed_token);
}

void WaveDrawer::doDraw(mono::IRenderer& renderer) const
{
    constexpr math::Matrix identity;
    const math::Matrix& projection = math::Ortho(0.0f, 200.0f, 0.0f, 100.0f, -10.0f, 10.0f);    

    renderer.PushNewTransform(identity);
    renderer.PushNewProjection(projection);

    constexpr math::Vector position(100.0f, 90.0f);
    renderer.DrawText(game::FontId::PIXELETTE_LARGE, m_current_text.c_str(), position, true, mono::Color::RED);
}

math::Quad WaveDrawer::BoundingBox() const
{
    return math::InfQuad;
}

mono::EventResult WaveDrawer::WaveStarted(const WaveStartedEvent& event)
{
    char buffer[64] = { '\0' };
    std::snprintf(buffer, 64, "Wave %d, %s", event.wave_index +1, event.wave_name);
    m_current_text = buffer;
    return mono::EventResult::PASS_ON;
}

mono::EventResult WaveDrawer::HordeCompleted(const HordeCompletedEvent& event)
{
    m_current_text = "Horde Completed!";
    return mono::EventResult::PASS_ON;
}
