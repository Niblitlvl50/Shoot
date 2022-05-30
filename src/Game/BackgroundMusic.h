
#pragma once

namespace game
{
    enum MusicTrack : int
    {
        None = 0,
        RussianTrack,
        Song18,
        WindsOfStories,
        Level3,
        Level4,
        Boss1,

        N_MUSIC_TRACKS
    };

    constexpr const char* g_music_track_strings[] = {
        "None",
        "RussianTrack",
        "Son18",
        "WindsOfStories",
        "Level3",
        "Level4",
        "Boss1",
    };

    constexpr const char* MusicTrackToString(MusicTrack track)
    {
        return g_music_track_strings[static_cast<int>(track)];
    }

    void InitializeBackgroundMusic();
    void ShutdownBackgroundMusic();

    void PlayBackgroundMusic(MusicTrack track);
    void StopBackgroundMusic();
}
