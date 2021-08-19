
#include "RegionDrawer.h"

#include "FontIds.h"
#include "TriggerSystem/TriggerSystem.h"

#include "Math/EasingFunctions.h"
#include "Rendering/RenderSystem.h"
#include "System/Hash.h"

#include <unordered_map>

namespace
{
    const uint32_t enter_death_valley_hash = hash::Hash("enter_death_valley");
    const uint32_t enter_oak_forest_hash = hash::Hash("enter_oak_forest");


    const std::unordered_map<uint32_t, const char*> g_hash_to_text = {
        { enter_death_valley_hash,  "Death Valley" },
        { enter_oak_forest_hash,    "Oak Forest" },
    };
}

using namespace game;

RegionDrawer::RegionDrawer(TriggerSystem* trigger_system)
    : UIOverlay(1200.0f, 1200.0f / mono::GetWindowAspect())
    , m_trigger_system(trigger_system)
    , m_color(mono::Color::BLACK)
{
    const game::TriggerCallback handle_region_trigger = [this](uint32_t trigger_hash) {
        HandleRegionTrigger(trigger_hash);
    };

    m_enter_death_valley_id = m_trigger_system->RegisterTriggerCallback(enter_death_valley_hash, handle_region_trigger, -1);
    m_enter_oak_forest_id = m_trigger_system->RegisterTriggerCallback(enter_oak_forest_hash, handle_region_trigger, -1);

    m_region_text = new UITextElement(shared::FontId::RUSSOONE_MEGA, "", mono::FontCentering::DEFAULT_CENTER, m_color);
    m_region_text->SetPosition(100, 700);
    AddChild(m_region_text);
}

RegionDrawer::~RegionDrawer()
{
    m_trigger_system->RemoveTriggerCallback(enter_death_valley_hash, m_enter_death_valley_id, -1);
    m_trigger_system->RemoveTriggerCallback(enter_oak_forest_hash, m_enter_oak_forest_id, -1);
}

void RegionDrawer::Update(const mono::UpdateContext& context)
{
    UIOverlay::Update(context);
    m_text_timer += context.delta_s;

    constexpr float ease_in_out_time = 1.0f;
    constexpr float display_time = 3.0f;
    constexpr float ease_out_time = ease_in_out_time + display_time;

    if(m_text_timer < ease_in_out_time)
    {
        m_color.alpha = math::EaseInOutCubic(m_text_timer, ease_in_out_time, 0.0f, 1.0f);
        m_region_text->SetColor(m_color);
    }
    else if(m_text_timer > display_time && m_text_timer < ease_out_time)
    {
        const float time = m_text_timer - display_time;
        m_color.alpha = math::EaseInOutCubic(time, 1.0f, 1.0f, -1.0f);
        m_region_text->SetColor(m_color);
    }
}

void RegionDrawer::HandleRegionTrigger(uint32_t trigger_hash)
{
    const auto it = g_hash_to_text.find(trigger_hash);
    if(it != g_hash_to_text.end())
    {
        m_region_text->SetText(it->second);
        m_text_timer = 0.0f;
    }
}
