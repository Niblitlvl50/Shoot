
#include "SoundSystem.h"
#include "TriggerSystem/TriggerSystem.h"

#include "System/Audio.h"
#include "System/Hash.h"

#include "Math/EasingFunctions.h"

#include "Debug/GameDebugVariables.h"
#include "Debug/IDebugDrawer.h"
#include "Rendering/Color.h"

#include "System/File.h"

#include "nlohmann/json.hpp"

#include <string>
#include <limits>

namespace tweak_values
{
    constexpr float fade_time_s = 1.0f;
}

namespace
{
    constexpr uint32_t NO_CALLBACK_SET = std::numeric_limits<uint32_t>::max();
}

using namespace game;

SoundSystem::SoundSystem(uint32_t n, mono::TriggerSystem* trigger_system)
    : m_trigger_system(trigger_system)
    , m_sound_components(n)
    , m_master_volume(1.0f)
    , m_current_track(0)
    , m_requested_track(0)
    , m_current_transition(SoundTransition::Cut)
    , m_transition_timer(0.0f)
{
    const std::vector<byte> file_data = file::FileReadAll("res/configs/sound_config.json");
    if(!file_data.empty())
    {
        const nlohmann::json& json = nlohmann::json::parse(file_data);
        m_master_volume = json["master_volume"];
        
        for(const auto& json_music_track : json["music_tracks"])
        {
            const std::string name = json_music_track["name"];
            const std::string filename = json_music_track["filename"];
            m_music_tracks[hash::Hash(name.c_str())] = audio::CreateSound(filename.c_str(), audio::SoundPlayback::LOOPING);

            hash::HashRegisterString(name.c_str());
        }
    }

    m_music_tracks[0] = audio::CreateNullSound();
}

SoundSystem::~SoundSystem()
{
    m_music_tracks.clear();
}

void SoundSystem::PlayBackgroundMusic(const std::string& name, SoundTransition transition)
{
    const uint32_t name_id = name.empty() ? 0 : hash::Hash(name.c_str());
    PlayBackgroundMusic(name_id, transition);
}

void SoundSystem::PlayBackgroundMusic(uint32_t track, SoundTransition transition)
{
    if(track == m_current_track)
        return;

    if(m_current_transition != SoundTransition::None)
    {
        // We are already in a transition
        m_music_tracks[m_requested_track]->Stop();
    }

    m_requested_track = track;
    m_current_transition = transition;
    m_transition_timer = 0.0f;

    audio::ISoundPtr& requested_track = m_music_tracks[m_requested_track];

    switch(transition)
    {
    case SoundTransition::None:
        break;

    case SoundTransition::Cut:
        requested_track->SetVolume(m_master_volume);
        requested_track->Play();
        break;

    case SoundTransition::CrossFade:
        requested_track->SetVolume(0.0f);
        requested_track->Play();
        break;

    case SoundTransition::FadeOutFadeIn:
        requested_track->SetVolume(0.0f);
        break;
    }
}

void SoundSystem::StopBackgroundMusic()
{
    PlayBackgroundMusic(0, SoundTransition::CrossFade);
}

SoundInstanceComponent* SoundSystem::AllocateSoundComponent(uint32_t entity_id)
{
    SoundInstanceComponent component;
    component.play_trigger = 0;
    component.stop_trigger = 0;
    component.play_callback_id = NO_CALLBACK_SET;
    component.stop_callback_id = NO_CALLBACK_SET;

    return m_sound_components.Set(entity_id, std::move(component));
}

void SoundSystem::ReleaseSoundComponent(uint32_t entity_id)
{
    SoundInstanceComponent* component = m_sound_components.Get(entity_id);
    if(component->play_callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(component->play_trigger, component->play_callback_id, entity_id);

    if(component->stop_callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(component->stop_trigger, component->stop_callback_id, entity_id);

    m_sound_components.Release(entity_id);
}

void SoundSystem::SetSoundComponentData(
    uint32_t entity_id, const std::string& sound_file, SoundInstancePlayParameter play_parameters, uint32_t play_trigger, uint32_t stop_trigger)
{
    const audio::SoundPlayback playback_param =
        (play_parameters & SoundInstancePlayParameter::SP_LOOPING) ? audio::SoundPlayback::LOOPING : audio::SoundPlayback::ONCE;

    SoundInstanceComponent* component = m_sound_components.Get(entity_id);
    component->sound = audio::CreateSound(sound_file.c_str(), playback_param);
    component->play_trigger = play_trigger;
    component->stop_trigger = stop_trigger;

    if(component->play_callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(component->play_trigger, component->play_callback_id, entity_id);

    if(component->stop_callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(component->stop_trigger, component->stop_callback_id, entity_id);

    const mono::TriggerCallback play_callback = [component](uint32_t trigger_id) {
        component->sound->Play();
    };
    component->play_callback_id = m_trigger_system->RegisterTriggerCallback(component->play_trigger, play_callback, entity_id);

    const mono::TriggerCallback stop_callback = [component](uint32_t trigger_id) {
        component->sound->Stop();
    };
    component->stop_callback_id = m_trigger_system->RegisterTriggerCallback(component->stop_trigger, stop_callback, entity_id);

    if(play_parameters & SoundInstancePlayParameter::SP_ACTIVE_ON_LOAD)
    {
        component->sound->Play();
    }
}

const char* SoundSystem::Name() const
{
    return "SoundSystem";
}

void SoundSystem::Update(const mono::UpdateContext& update_context)
{
    if(game::g_draw_debug_soundsystem)
        DrawDebug(update_context);
    
    audio::ISoundPtr& current_track = m_music_tracks[m_current_track];
    audio::ISoundPtr& requested_track = m_music_tracks[m_requested_track];

    m_transition_timer = std::clamp(m_transition_timer + update_context.delta_s, 0.0f, tweak_values::fade_time_s);
    
    if(m_current_transition != SoundTransition::None)
    {
        switch(m_current_transition)
        {
        case SoundTransition::None:
            break;
        case SoundTransition::Cut:
            break;

        case SoundTransition::CrossFade:
        {
            const float new_value = math::LinearTween(m_transition_timer, tweak_values::fade_time_s, 0.0f, m_master_volume);
            current_track->SetVolume(m_master_volume - new_value);
            requested_track->SetVolume(new_value);

            break;
        }

        case SoundTransition::FadeOutFadeIn:
        {
            const float new_value = math::LinearTween(m_transition_timer, tweak_values::fade_time_s, 0.0f, m_master_volume * 2.0f);
            if(new_value <= m_master_volume)
                current_track->SetVolume(m_master_volume - new_value);
            else
                requested_track->SetVolume(new_value - m_master_volume);

            break;
        }
        }

        if(m_transition_timer >= tweak_values::fade_time_s)
        {
            if(m_current_track != m_requested_track)
                current_track->Stop();
            m_current_track = m_requested_track;
            m_current_transition = SoundTransition::None;
        }
    }

    if(game::g_mute_soundsystem)
    {
        current_track->SetVolume(0.0f);
        requested_track->SetVolume(0.0f);
    }
}

void SoundSystem::DrawDebug(const mono::UpdateContext& update_context)
{
    const std::string muted_text = game::g_mute_soundsystem ? " | Muted" : "";
    const std::string text =
        hash::HashLookup(m_current_track) + std::string(" -> ") + hash::HashLookup(m_requested_track) + muted_text;
    g_debug_drawer->DrawScreenText(text.c_str(), math::Vector(0.5f, 1.0f), mono::Color::CYAN);

    const std::string transision_string =
        SoundTransisionToString(m_current_transition) + std::string(" | ") + std::to_string(m_transition_timer);
    g_debug_drawer->DrawScreenText(transision_string.c_str(), math::Vector(0.5f, 0.5f), mono::Color::CYAN);
}
