
#pragma once

#include "MonoFwd.h"

#include "Camera/CameraController.h"
#include "Events/EventFwd.h"
#include "Math/MathFwd.h"
#include "IGameSystem.h"
#include "Util/ActiveVector.h"

#include <vector>
#include <stack>

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
        uint32_t entity_id;

        union
        {
            float zoom_level;
        };
    };

    struct CameraRestoreComponent
    {
        uint32_t trigger_hash;
        uint32_t callback_id;
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

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;
        void Reset() override;

        void FollowEntity(uint32_t entity_id);
        void FollowEntityOffset(const math::Vector& offset);
        void Unfollow(uint32_t entity_id);

        CameraAnimationComponent* AllocateCameraAnimation(uint32_t entity_id);
        void ReleaseCameraAnimation(uint32_t entity_id);
        void AddCameraAnimationComponent(uint32_t entity_id, uint32_t trigger_hash, float new_zoom_level);
        void AddCameraAnimationComponent(uint32_t entity_id, uint32_t trigger_hash);
        void AddCameraAnimationComponent(uint32_t entity_id, uint32_t trigger_hash, uint32_t follow_entity_id);

        CameraRestoreComponent* AllocateRestoreComponent(uint32_t entity_id);
        void ReleaseRestoreComponent(uint32_t entity_id);
        void AddRestoreComponent(uint32_t entity_id, uint32_t trigger_hash);

        void AddCameraShake(float time_s);

        const mono::ICamera* GetActiveCamera() const;
        mono::ICamera* GetActiveCamera();

private:

        void PushCameraData(uint32_t debug_entity_id);
        void PopCameraData();

        mono::ICamera* m_camera;
        mono::TransformSystem* m_transform_system;
        class TriggerSystem* m_trigger_system;

        mono::MouseCameraController m_controller;

        math::Vector m_current_camera_size;
        std::vector<uint32_t> m_follow_entities;
        math::Vector m_current_follow_offset;

        struct CameraStackData
        {
            math::Vector camera_size;
            std::vector<uint32_t> follow_entities;
            math::Vector follow_offset;
            uint32_t debug_entity_id;
        };
        std::stack<CameraStackData> m_camera_stack;

        float m_camera_shake_timer_s;

        mono::ActiveVector<CameraAnimationComponent> m_animation_components;
        mono::ActiveVector<CameraRestoreComponent> m_restore_components;
        std::vector<CameraAnimationComponent*> m_camera_anims_to_process;
    };
}
