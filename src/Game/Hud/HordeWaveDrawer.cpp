
#include "HordeWaveDrawer.h"
#include "FontIds.h"

#include "Math/EasingFunctions.h"
#include "Rendering/RenderSystem.h"

using namespace game;

HordeWaveInfoElement::HordeWaveInfoElement()
    : UISquareElement(15.0f, 4.0f, mono::Color::MakeWithAlpha(mono::Color::DIM_GRAY, 0.25f))
{
    SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    const math::Vector anchor_offset = GetAnchorOffset();

    m_wave_text = new UITextElement(FontId::RUSSOONE_LARGE, "", mono::Color::GOLDEN_YELLOW);
    m_wave_text->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    m_wave_text->SetPosition(-anchor_offset + math::Vector(0.5f, 2.5f));

    m_wave_subtext = new UITextElement(FontId::RUSSOONE_SMALL, "", mono::Color::BLUE_GRAY);
    m_wave_subtext->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    m_wave_subtext->SetPosition(-anchor_offset + math::Vector(0.75f, 1.0f));

    m_wave_subtext_description = new UITextElement(FontId::RUSSOONE_TINY, "", mono::Color::BLUE_GRAY);
    m_wave_subtext_description->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    m_wave_subtext_description->SetPosition(-anchor_offset + math::Vector(0.75f, 0.5f));     

    m_icon = new UISpriteElement("res/sprites/powerup_vampiric_icon.sprite");
    m_icon->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    m_icon->SetPosition(-anchor_offset + math::Vector(8.0f, 2.0f));
    m_icon->SetScale(6.0f);

    AddChild(m_wave_text);
    AddChild(m_wave_subtext);
    AddChild(m_wave_subtext_description);
    AddChild(m_icon);
}

void HordeWaveInfoElement::SetWaveInfo(int wave_index, const std::string& sub_text, const std::string& sub_text_description, const std::string& sprite_file)
{
    const std::string wave_text = "Wave " + std::to_string(wave_index);
    m_wave_text->SetText(wave_text);
    m_wave_subtext->SetText(sub_text);
    m_wave_subtext_description->SetText(sub_text_description);
    m_icon->SetSprite(sprite_file);
}

HordeWaveDrawer::HordeWaveDrawer()
    : UIOverlay(50.0f, 50.0f / mono::RenderSystem::GetWindowAspect())
    , m_text_timer_s(1000.0f)
{
    m_onscreen_position = math::Vector(40.0f, m_height - 8.0f);
    m_offscreen_position = m_onscreen_position + math::Vector(10.0f, 0.0f);

    m_wave_info_element = new HordeWaveInfoElement();
    m_wave_info_element->SetPosition(m_offscreen_position);

    AddChild(m_wave_info_element);
}

void HordeWaveDrawer::Update(const mono::UpdateContext& update_context)
{
    UIOverlay::Update(update_context);
    m_text_timer_s += update_context.delta_s;

    constexpr float transition_duration_s = 0.8f;
    constexpr float ease_in_out_time_s = 1.0f;
    constexpr float display_time_s = 15.0f;
    constexpr float ease_out_time_s = ease_in_out_time_s + display_time_s;

    math::Vector background_position = m_wave_info_element->GetPosition();

    if(m_text_timer_s < ease_in_out_time_s)
    {
        background_position.x = math::EaseInOutCubic(m_text_timer_s, transition_duration_s, m_offscreen_position.x, m_onscreen_position.x - m_offscreen_position.x);
    }
    else if(m_text_timer_s > display_time_s && m_text_timer_s < ease_out_time_s)
    {
        const float time_s = m_text_timer_s - display_time_s;
        background_position.x = math::EaseInOutCubic(time_s, transition_duration_s, m_onscreen_position.x, m_offscreen_position.x - m_onscreen_position.x);
    }

    m_wave_info_element->SetPosition(background_position);
}

void HordeWaveDrawer::ShowNextWave(int wave_index, const std::string& sub_text, const std::string& sub_text_description, const std::string& sprite_file)
{
    m_text_timer_s = 0.0f;
    m_wave_info_element->SetWaveInfo(wave_index, sub_text, sub_text_description, sprite_file);
}
