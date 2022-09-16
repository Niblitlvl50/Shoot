
#include "SoundSystem.h"
#include "TriggerSystem/TriggerSystem.h"

#include "System/Audio.h"
#include "System/Hash.h"

#include "Math/EasingFunctions.h"

#include "GameDebug.h"
#include "Factories.h"
#include "IDebugDrawer.h"
#include "Rendering/Color.h"

#include <string>
#include <limits>

namespace tweak_values
{
    constexpr float fade_time_s = 2.0f;
}

namespace
{
    constexpr uint32_t NO_CALLBACK_SET = std::numeric_limits<uint32_t>::max();
}

using namespace game;

SoundSystem::SoundSystem(uint32_t n, game::TriggerSystem* trigger_system)
    : m_trigger_system(trigger_system)
    , m_sound_components(n)
    , m_current_track(MusicTrack::None)
    , m_requested_track(MusicTrack::None)
    , m_current_transition(SoundTransition::Cut)
    , m_transition_timer(0.0f)
{
    m_music_tracks[game::None] = audio::CreateNullSound();
    m_music_tracks[game::RussianTrack] = audio::CreateSound("res/sound/background_music/russian_track.ogg", audio::SoundPlayback::LOOPING);
    m_music_tracks[game::Song18] = audio::CreateSound("res/sound/background_music/song18.ogg", audio::SoundPlayback::LOOPING);
    m_music_tracks[game::WindsOfStories] = audio::CreateSound("res/sound/background_music/winds_of_stories.ogg", audio::SoundPlayback::LOOPING);
    m_music_tracks[game::Level3] = audio::CreateSound("res/sound/background_music/level3.ogg", audio::SoundPlayback::LOOPING);
    m_music_tracks[game::Level4] = audio::CreateSound("res/sound/background_music/level4.ogg", audio::SoundPlayback::LOOPING);
    m_music_tracks[game::Boss1] = audio::CreateSound("res/sound/background_music/boss1.ogg", audio::SoundPlayback::LOOPING);
}

SoundSystem::~SoundSystem()
{
    for(audio::ISoundPtr& sound : m_music_tracks)
        sound = nullptr;
}

void SoundSystem::PlayBackgroundMusic(MusicTrack track, SoundTransition transition)
{
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
        requested_track->SetVolume(1.0f);
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
    PlayBackgroundMusic(MusicTrack::None, SoundTransition::CrossFade);
}

SoundInstanceComponent* SoundSystem::AllocateSoundComponent(uint32_t entity_id)
{
    SoundInstanceComponent component;
    component.callback_id = NO_CALLBACK_SET;

    return m_sound_components.Set(entity_id, std::move(component));
}

void SoundSystem::ReleaseSoundComponent(uint32_t entity_id)
{
    SoundInstanceComponent* component = m_sound_components.Get(entity_id);
    if(component->callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(component->play_trigger, component->callback_id, entity_id);

    m_sound_components.Release(entity_id);
}

void SoundSystem::SetSoundComponentData(
    uint32_t entity_id, const std::string& sound_file, SoundInstancePlayParameter play_parameters, uint32_t play_trigger)
{
    const audio::SoundPlayback playback_param =
        (play_parameters & SoundInstancePlayParameter::SP_LOOPING) ? audio::SoundPlayback::LOOPING : audio::SoundPlayback::ONCE;

    SoundInstanceComponent* component = m_sound_components.Get(entity_id);
    component->sound = audio::CreateSound(sound_file.c_str(), playback_param);
    component->play_trigger = play_trigger;

    if(component->callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(component->play_trigger, component->callback_id, entity_id);

    if(play_parameters & SoundInstancePlayParameter::SP_TRIGGER_ACTIVATED)
    {
        const TriggerCallback callback = [component](uint32_t trigger_id) {
            component->sound->Play();
        };
        component->callback_id = m_trigger_system->RegisterTriggerCallback(component->play_trigger, callback, entity_id);
    }

    if(play_parameters & SoundInstancePlayParameter::SP_ACTIVE_ON_LOAD)
    {
        component->sound->Play();
    }
}

uint32_t SoundSystem::Id() const
{
    return hash::Hash(Name());
}

const char* SoundSystem::Name() const
{
    return "SoundSystem";
}

void SoundSystem::Update(const mono::UpdateContext& update_context)
{
    if(game::g_draw_debug_soundsystem)
    {
        std::string text = MusicTrackToString(m_current_track) + std::string(" -> ") + MusicTrackToString(m_requested_track);
        g_debug_drawer->DrawScreenText(text.c_str(), math::Vector(0.5f, 1.0f), mono::Color::RED);
    }
    
    if(m_current_transition == SoundTransition::None)
        return;

    m_transition_timer = std::clamp(m_transition_timer + update_context.delta_s, 0.0f, tweak_values::fade_time_s);

    audio::ISoundPtr& current_track = m_music_tracks[m_current_track];
    audio::ISoundPtr& requested_track = m_music_tracks[m_requested_track];

    switch(m_current_transition)
    {
    case SoundTransition::None:
        break;
    case SoundTransition::Cut:
        break;

    case SoundTransition::CrossFade:
    {
        const float new_value = math::LinearTween(m_transition_timer, tweak_values::fade_time_s, 0.0f, 1.0f);
        current_track->SetVolume(1.0f - new_value);
        requested_track->SetVolume(new_value);

        break;
    }

    case SoundTransition::FadeOutFadeIn:
    {
        const float new_value = math::LinearTween(m_transition_timer, tweak_values::fade_time_s, 0.0f, 2.0f);
        if(new_value < 1.0f)
        {
            current_track->SetVolume(1.0f - new_value);
        }
        else
        {
            if(!requested_track->IsPlaying())
                requested_track->Play();
            requested_track->SetVolume(new_value - 1.0f);
        }

        break;
    }
    }

    if(m_transition_timer >= tweak_values::fade_time_s)
    {
        current_track->Stop();
        m_current_track = m_requested_track;
        m_current_transition = SoundTransition::None;
    }
}
