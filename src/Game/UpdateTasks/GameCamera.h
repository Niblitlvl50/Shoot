
#pragma once

#include "MonoFwd.h"
#include "IUpdatable.h"
#include "Camera/CameraController.h"

#include "EventHandler/EventToken.h"
#include "Events/EventFwd.h"

namespace mono
{
    class TransformSystem;
}

namespace game
{
    class GameCamera : public mono::IUpdatable
    {
    public:
        GameCamera(mono::ICamera* camera, mono::TransformSystem* transform_system, mono::EventHandler& event_handler);
        ~GameCamera();

        void Update(const mono::UpdateContext& update_context) override;

        void Follow(uint32_t entity_id, const math::Vector& offset);
        void Unfollow();

    private:
        mono::ICamera* m_camera;
        mono::TransformSystem* m_transform_system;
        mono::EventHandler& m_event_handler;
        mono::MouseCameraController m_controller;

        math::Vector m_offset;
        bool m_debug_camera;
        uint32_t m_entity_id;

        mono::EventToken<event::KeyDownEvent> m_key_down_token;
    };
}
