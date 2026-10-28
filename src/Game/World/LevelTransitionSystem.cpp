
#include "LevelTransitionSystem.h"
#include "Zones/ZoneManager.h"

#include "EntitySystem/Entity.h"
#include "EntitySystem/IEntityManager.h"
#include "EventHandler/EventHandler.h"
#include "Events/QuitEvent.h"
#include "Rendering/RenderSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "System/Hash.h"
#include "System/System.h"

#include <algorithm>

namespace tweak_values
{
    constexpr float fade_out_duration_s = 1.0f;
}

using namespace game;

LevelTransitionSystem::LevelTransitionSystem(
    mono::TriggerSystem* trigger_system,
    mono::RenderSystem* render_system,
    mono::TransformSystem* transform_system,
    mono::IEntityManager* entity_manager,
    mono::EventHandler* event_handler)
    : m_trigger_system(trigger_system)
    , m_render_system(render_system)
    , m_transform_system(transform_system)
    , m_entity_manager(entity_manager)
    , m_event_handler(event_handler)
    , m_exiting(false)
{ }

void LevelTransitionSystem::AllocateLevelExit(uint32_t entity_id)
{
    m_level_exits[entity_id] = { hash::NO_HASH, mono::INVALID_ID, std::string(), std::string() };
}

void LevelTransitionSystem::ReleaseLevelExit(uint32_t entity_id)
{
    const auto it = m_level_exits.find(entity_id);
    if(it == m_level_exits.end())
        return;

    if(it->second.trigger_handle != mono::INVALID_ID)
        m_trigger_system->RemoveTriggerCallback(it->second.trigger_hash, it->second.trigger_handle, entity_id);

    m_level_exits.erase(it);
}

void LevelTransitionSystem::SetLevelExitData(
    uint32_t entity_id, uint32_t trigger_hash, const std::string& target_level, const std::string& entry_tag)
{
    const auto it = m_level_exits.find(entity_id);
    if(it == m_level_exits.end())
        return;

    LevelExit& level_exit = it->second;
    if(level_exit.trigger_handle != mono::INVALID_ID)
        m_trigger_system->RemoveTriggerCallback(level_exit.trigger_hash, level_exit.trigger_handle, entity_id);

    level_exit.trigger_hash = trigger_hash;
    level_exit.trigger_handle = mono::INVALID_ID;
    level_exit.target_level = target_level;
    level_exit.entry_tag = entry_tag;

    if(trigger_hash != hash::NO_HASH)
    {
        const mono::TriggerCallback exit_callback = [this, entity_id](uint32_t trigger_id) {
            ExitLevel(entity_id);
        };
        level_exit.trigger_handle = m_trigger_system->RegisterTriggerCallback(trigger_hash, exit_callback, entity_id);
    }
}

math::Vector LevelTransitionSystem::GetPlayerSpawnPoint(const math::Vector& default_spawn_point)
{
    const std::string entry_tag = m_pending_entry_tag;
    m_pending_entry_tag.clear();

    if(entry_tag.empty())
        return default_spawn_point;

    const std::vector<uint32_t> entry_entities = m_entity_manager->CollectEntitiesWithTag(hash::Hash(entry_tag.c_str()));
    if(entry_entities.empty())
    {
        System::Log("LevelTransitionSystem|No entity with tag '%s', using the level's spawn point.", entry_tag.c_str());
        return default_spawn_point;
    }

    return m_transform_system->GetWorldPosition(entry_entities.front());
}

void LevelTransitionSystem::ExitLevel(uint32_t entity_id)
{
    if(m_exiting)
        return;

    const auto it = m_level_exits.find(entity_id);
    if(it == m_level_exits.end())
        return;

    const LevelExit& level_exit = it->second;

    const std::vector<Level> levels = ZoneManager::GetLevels();
    const auto find_level = [&level_exit](const Level& level) {
        return level.name == level_exit.target_level;
    };
    const auto level_it = std::find_if(levels.begin(), levels.end(), find_level);
    if(level_it == levels.end())
    {
        System::Log("LevelTransitionSystem|Unable to find level '%s' in the level config.", level_exit.target_level.c_str());
        return;
    }

    m_exiting = true;
    m_pending_entry_tag = level_exit.entry_tag;
    ZoneManager::SwitchToLevel(*level_it);

    const mono::ScreenFadeCallback on_fade_complete = [this](mono::ScreenFadeState state) {
        m_event_handler->DispatchEvent(event::QuitEvent());
    };
    m_render_system->TriggerScreenFade(mono::ScreenFadeState::FADE_OUT, tweak_values::fade_out_duration_s, 0.0f, on_fade_complete);
}

const char* LevelTransitionSystem::Name() const
{
    return "leveltransitionsystem";
}

void LevelTransitionSystem::Reset()
{
    m_exiting = false;
}

void LevelTransitionSystem::Update(const mono::UpdateContext& update_context)
{ }
