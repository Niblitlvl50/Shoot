
#include "LevelTimerUIElement.h"
#include "FontIds.h"
#include "Rendering/RenderSystem.h"

using namespace game;

LevelTimerUIElement::LevelTimerUIElement()
    : UIOverlay(50.0f, 50.0f / mono::RenderSystem::GetWindowAspect())
    , m_seconds(0)
{
    m_time_element = new UITextElement(
        game::FontId::RUSSOONE_LARGE, "", mono::FontCentering::DEFAULT_CENTER, mono::Color::OFF_WHITE);
    m_time_element->SetPosition(25.0f, m_height - 2.0f);
    AddChild(m_time_element);
}

void LevelTimerUIElement::Update(const mono::UpdateContext& context)
{
    UIOverlay::Update(context);

    const int seconds = int(m_seconds) % 60;
    const int minutes = m_seconds / 60.0f;
    //const float hours = minutes / 60.0f;

    char text_buffer[1024] = {};
    std::snprintf(text_buffer, std::size(text_buffer), "%d:%.2d", minutes, seconds);
    m_time_element->SetText(text_buffer);
}

void LevelTimerUIElement::SetSeconds(int seconds)
{
    m_seconds = seconds;
}
