
#include "HordeWaveDrawer.h"
#include "FontIds.h"

#include "Math/EasingFunctions.h"
#include "Rendering/RenderSystem.h"

using namespace game;

HordeWaveDrawer::HordeWaveDrawer()
    : UIOverlay(50.0f, 50.0f / mono::RenderSystem::GetWindowAspect())
    , m_text_timer_s(1000.0f)
{
    m_onscreen_position = math::Vector(40.0f, m_height - 8.0f);
    m_offscreen_position = m_onscreen_position + math::Vector(10.0f, 0.0f);

    m_background = new UISquareElement(15.0f, 3.5f, mono::Color::MakeWithAlpha(mono::Color::GRAY, 0.25f));
    m_background->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    m_background->SetPosition(m_offscreen_position);

    const math::Vector anchor_offset = m_background->GetAnchorOffset();

    m_wave_text = new UITextElement(FontId::RUSSOONE_LARGE, "", mono::Color::GOLDEN_YELLOW);
    m_wave_text->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    m_wave_text->SetPosition(-anchor_offset + math::Vector(0.5f, 2.0f));

    m_wave_subtext = new UITextElement(FontId::RUSSOONE_MEDIUM, "", mono::Color::LIGHT_GRAY);
    m_wave_subtext->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    m_wave_subtext->SetPosition(-anchor_offset + math::Vector(0.75f, 0.5f));

    m_background->AddChild(m_wave_text);
    m_background->AddChild(m_wave_subtext);

    AddChild(m_background);
}

void HordeWaveDrawer::Update(const mono::UpdateContext& update_context)
{
    UIOverlay::Update(update_context);
    m_text_timer_s += update_context.delta_s;

    constexpr float transition_duration_s = 0.8f;
    constexpr float ease_in_out_time_s = 1.0f;
    constexpr float display_time_s = 3.0f;
    constexpr float ease_out_time_s = ease_in_out_time_s + display_time_s;

    math::Vector background_position = m_background->GetPosition();

    if(m_text_timer_s < ease_in_out_time_s)
    {
        background_position.x = math::EaseInOutCubic(m_text_timer_s, transition_duration_s, m_offscreen_position.x, m_onscreen_position.x - m_offscreen_position.x);
    }
    else if(m_text_timer_s > display_time_s && m_text_timer_s < ease_out_time_s)
    {
        const float time_s = m_text_timer_s - display_time_s;
        background_position.x = math::EaseInOutCubic(time_s, transition_duration_s, m_onscreen_position.x, m_offscreen_position.x - m_onscreen_position.x);
    }

    m_background->SetPosition(background_position);
}

void HordeWaveDrawer::ShowNextWave(int wave_index, const std::string& sub_text)
{
    m_text_timer_s = 0.0f;

    const std::string wave_text = "Wave " + std::to_string(wave_index);
    m_wave_text->SetText(wave_text);
    m_wave_subtext->SetText(sub_text);
}
