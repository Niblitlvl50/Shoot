
#include "SoundSystem.h"
#include "System/Audio.h"
#include "System/Hash.h"

using namespace game;

SoundSystem::SoundSystem()
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

void SoundSystem::PlayBackgroundMusic(MusicTrack track)
{
    m_music_tracks[track]->Play();
}

void SoundSystem::StopBackgroundMusic()
{
    for(audio::ISoundPtr& sound : m_music_tracks)
        sound->Stop();
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

}
