
#pragma once

#include "IGameSystem.h"
#include "System/Audio.h"

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

    enum class SoundTransition
    {
        None,
        Cut,
        CrossFade,
        FadeOutFadeIn
    };

    class SoundSystem : public mono::IGameSystem
    {
    public:

        SoundSystem();
        ~SoundSystem();

        void PlayBackgroundMusic(MusicTrack track, SoundTransition transition);
        void StopBackgroundMusic();

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        audio::ISoundPtr m_music_tracks[game::N_MUSIC_TRACKS];

        MusicTrack m_current_track;
        MusicTrack m_requested_track;

        SoundTransition m_current_transition;
        float m_transition_timer; 
    };
}
