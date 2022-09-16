
#pragma once

#include "IGameSystem.h"
#include "Util/ActiveVector.h"
#include "System/Audio.h"

#include <string>

#define ENUM_BIT(n) (1 << (n))

namespace game
{
    class TriggerSystem;

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

    enum SoundInstancePlayParameter
    {
        SP_TRIGGER_ACTIVATED   = ENUM_BIT(0),
        SP_ACTIVE_ON_LOAD      = ENUM_BIT(1),
        SP_LOOPING             = ENUM_BIT(2)
    };

    static const std::vector<uint32_t> all_sound_play_parameters = {
        SoundInstancePlayParameter::SP_TRIGGER_ACTIVATED,
        SoundInstancePlayParameter::SP_ACTIVE_ON_LOAD,
        SoundInstancePlayParameter::SP_LOOPING,
    };

    inline const char* SoundInstancePlayParamterToString(uint32_t parameter)
    {
        if(parameter == SP_TRIGGER_ACTIVATED)
            return "Trigger Activated";
        else if(parameter == SP_ACTIVE_ON_LOAD)
            return "Play on Load";
        else if(parameter == SP_LOOPING)
            return "Looping";

        return "Unknown";
    }

    struct SoundInstanceComponent
    {
        audio::ISoundPtr sound;
        uint32_t play_trigger;
        uint32_t callback_id;
    };

    class SoundSystem : public mono::IGameSystem
    {
    public:

        SoundSystem(uint32_t n, game::TriggerSystem* trigger_system);
        ~SoundSystem();

        void PlayBackgroundMusic(MusicTrack track, SoundTransition transition);
        void StopBackgroundMusic();

        SoundInstanceComponent* AllocateSoundComponent(uint32_t entity_id);
        void ReleaseSoundComponent(uint32_t entity_id);
        void SetSoundComponentData(
            uint32_t entity_id, const std::string& sound_file, SoundInstancePlayParameter play_parameter, uint32_t play_trigger);

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        game::TriggerSystem* m_trigger_system;
        mono::ActiveVector<SoundInstanceComponent> m_sound_components;

        audio::ISoundPtr m_music_tracks[game::N_MUSIC_TRACKS];

        MusicTrack m_current_track;
        MusicTrack m_requested_track;

        SoundTransition m_current_transition;
        float m_transition_timer; 
    };
}
