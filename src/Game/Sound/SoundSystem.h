
#pragma once

#include "IGameSystem.h"
#include "Util/ActiveVector.h"
#include "System/Audio.h"

#include <string>
#include <unordered_map>

#define ENUM_BIT(n) (1 << (n))

namespace game
{
    class TriggerSystem;

    enum class SoundTransition
    {
        None,
        Cut,
        CrossFade,
        FadeOutFadeIn
    };

    inline const char* SoundTransisionToString(SoundTransition transision)
    {
        switch(transision)
        {
        case SoundTransition::None:
            return "None";
        case SoundTransition::Cut:
            return "Cut";
        case SoundTransition::CrossFade:
            return "CrossFade";
        case SoundTransition::FadeOutFadeIn:
            return "FadeOutFadeInt";
        }

        return "Unknown";
    }

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
        uint32_t stop_trigger;

        uint32_t play_callback_id;
        uint32_t stop_callback_id;
    };

    class SoundSystem : public mono::IGameSystem
    {
    public:

        SoundSystem(uint32_t n, game::TriggerSystem* trigger_system);
        ~SoundSystem();

        void PlayBackgroundMusic(const std::string& name, SoundTransition transition);
        void PlayBackgroundMusic(uint32_t track, SoundTransition transition);
        void StopBackgroundMusic();

        SoundInstanceComponent* AllocateSoundComponent(uint32_t entity_id);
        void ReleaseSoundComponent(uint32_t entity_id);
        void SetSoundComponentData(
            uint32_t entity_id, const std::string& sound_file, SoundInstancePlayParameter play_parameter, uint32_t play_trigger, uint32_t stop_trigger);

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void DrawDebug(const mono::UpdateContext& update_context);

        game::TriggerSystem* m_trigger_system;
        mono::ActiveVector<SoundInstanceComponent> m_sound_components;

        std::unordered_map<uint32_t, audio::ISoundPtr> m_music_tracks;

        uint32_t m_current_track;
        uint32_t m_requested_track;

        SoundTransition m_current_transition;
        float m_transition_timer;
    };
}
