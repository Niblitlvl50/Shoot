
#pragma once

namespace game
{
    enum MusicTrack
    {
        RussianTrack = 0,
        Song18,
        WindsOfStories,

        N_MUSIC_TRACKS
    };

    void InitializeBackgroundMusic();
    void ShutdownBackgroundMusic();

    void PlayBackgroundMusic(MusicTrack track);
    void StopBackgroundMusic();
}
