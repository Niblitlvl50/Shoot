
#pragma once

#include "MonoFwd.h"

#include "Camera/CameraController.h"
#include "EventHandler/EventToken.h"
#include "Events/EventFwd.h"
#include "Math/MathFwd.h"
#include "IGameSystem.h"

#include <vector>

namespace game
{
    enum class CameraAnimationType
    {
        ZOOM_LEVEL,
        CENTER_ON_POINT,
        CENTER_ON_ENTITY,
    };

    struct CameraAnimationComponent
    {
        uint32_t trigger_hash;
        uint32_t callback_id;
        CameraAnimationType type;

        union
        {
            float zoom_level;
            uint32_t entity_id;
            float point_x;
            float point_y;
        };
    };

    class CameraSystem : public mono::IGameSystem
    {
    public:

        CameraSystem(
            uint32_t n,
            mono::ICamera* camera,
            mono::TransformSystem* transform_system,
            mono::EventHandler* event_handler,
            class TriggerSystem* trigger_system);
        ~CameraSystem();

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void Follow(uint32_t entity_id, const math::Vector& offset);
        void Unfollow();

        CameraAnimationComponent* AllocateCameraAnimation(uint32_t entity_id);
        void ReleaseCameraAnimation(uint32_t entity_id);
        void AddCameraAnimationComponent(uint32_t entity_id, uint32_t trigger_hash, float new_zoom_level);
        void AddCameraAnimationComponent(uint32_t entity_id, uint32_t trigger_hash, const math::Vector& world_point);
        void AddCameraAnimationComponent(uint32_t entity_id, uint32_t trigger_hash, uint32_t follow_entity_id);

        void AddCameraShake(uint32_t time_ms);

        mono::ICamera* m_camera;
        mono::TransformSystem* m_transform_system;
        mono::EventHandler* m_event_handler;
        class TriggerSystem* m_trigger_system;

        mono::MouseCameraController m_controller;
        bool m_debug_camera;
        uint32_t m_entity_id;
        math::Vector m_follow_offset;

        int m_camera_shake_timer_ms;

        std::vector<CameraAnimationComponent> m_camera_components;
        std::vector<bool> m_active_camera_components;
        std::vector<CameraAnimationComponent*> m_camera_anims_to_process;

        mono::EventToken<event::KeyDownEvent> m_key_down_token;
    };
}
