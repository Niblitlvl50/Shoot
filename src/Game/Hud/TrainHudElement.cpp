
#include "TrainHudElement.h"
#include "FontIds.h"
#include "Rendering/RenderSystem.h"

#include <cstdio>
#include <cmath>

using namespace game;

TrainHudElement::TrainHudElement()
    : UIOverlay(50.0f, 50.0f / mono::RenderSystem::GetWindowAspect())
    , m_throttle(0.0f)
    , m_speed(0.0f)
{
    constexpr float bar_width = 10.0f;
    constexpr float bar_height = 0.6f;

    m_throttle_bar = new UIBarElement(bar_width, bar_height, mono::Color::GRAY, mono::Color::GREEN_VIVID);
    m_throttle_bar->SetPosition(1.0f, 1.0f);
    AddChild(m_throttle_bar);

    m_speed_text = new UITextElement(FontId::RUSSOONE_MEDIUM, "", mono::Color::OFF_WHITE);
    m_speed_text->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    m_speed_text->SetPosition(1.0f, 2.0f);
    AddChild(m_speed_text);
}

void TrainHudElement::Update(const mono::UpdateContext& context)
{
    UIOverlay::Update(context);

    const float throttle_fraction = std::fabs(m_throttle);
    m_throttle_bar->SetFraction(throttle_fraction);

    char speed_buffer[64] = {};
    std::snprintf(speed_buffer, std::size(speed_buffer), "%.1f km/h", m_speed * 3.6f);
    m_speed_text->SetText(speed_buffer);
}

void TrainHudElement::SetThrottle(float throttle)
{
    m_throttle = throttle;
}

void TrainHudElement::SetSpeed(float meters_per_second)
{
    m_speed = meters_per_second;
}
