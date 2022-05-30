
#include "BackgroundMusic.h"
#include "System/Audio.h"

namespace
{
    audio::ISoundPtr g_music_tracks[game::N_MUSIC_TRACKS];
}

void game::InitializeBackgroundMusic()
{
    g_music_tracks[game::None] = audio::CreateNullSound();
    g_music_tracks[game::RussianTrack] = audio::CreateSound("res/sound/background_music/russian_track.ogg", audio::SoundPlayback::LOOPING);
    g_music_tracks[game::Song18] = audio::CreateSound("res/sound/background_music/song18.ogg", audio::SoundPlayback::LOOPING);
    g_music_tracks[game::WindsOfStories] = audio::CreateSound("res/sound/background_music/winds_of_stories.ogg", audio::SoundPlayback::LOOPING);
    g_music_tracks[game::Level3] = audio::CreateSound("res/sound/background_music/level3.ogg", audio::SoundPlayback::LOOPING);
    g_music_tracks[game::Level4] = audio::CreateSound("res/sound/background_music/level4.ogg", audio::SoundPlayback::LOOPING);
    g_music_tracks[game::Boss1] = audio::CreateSound("res/sound/background_music/boss1.ogg", audio::SoundPlayback::LOOPING);
}

void game::ShutdownBackgroundMusic()
{
    for(audio::ISoundPtr& sound : g_music_tracks)
        sound = nullptr;
}

void game::PlayBackgroundMusic(game::MusicTrack track)
{
    g_music_tracks[track]->Play();
}

void game::StopBackgroundMusic()
{
    for(audio::ISoundPtr& sound : g_music_tracks)
        sound->Stop();
}

