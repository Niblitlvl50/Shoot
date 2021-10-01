
#include "BackgroundMusic.h"
#include "System/Audio.h"

namespace
{
    audio::ISoundPtr g_music_tracks[game::N_MUSIC_TRACKS];
}

void game::InitializeBackgroundMusic()
{
    g_music_tracks[game::RussianTrack] = audio::CreateSound("res/sound/background_music/russian_track.wav", audio::SoundPlayback::LOOPING);
    g_music_tracks[game::Song18] = audio::CreateSound("res/sound/background_music/song18.wav", audio::SoundPlayback::LOOPING);
    g_music_tracks[game::WindsOfStories] = audio::CreateSound("res/sound/background_music/winds_of_stories.wav", audio::SoundPlayback::LOOPING);
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

