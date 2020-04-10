
#pragma once

#include "Camera/ICamera.h"
#include "Camera/CameraController.h"
#include "MonoFwd.h"
#include "Math/Quad.h"

#include "EventHandler/EventToken.h"
#include "Events/EventFwd.h"

namespace mono
{
    class TransformSystem;
}

namespace game
{
    class Camera : public mono::ICamera
    {
    public:

        Camera(int width, int height, int window_width, int window_height, mono::TransformSystem* transform_system, mono::EventHandler& event_handler);
        ~Camera();

        mono::EventResult OnKeyDown(const event::KeyDownEvent& event);
        
        void doUpdate(const mono::UpdateContext& update_context) override;
        
        void Follow(uint32_t entity_id, const math::Vector& offset);
        void Unfollow();

        void SetViewport(const math::Quad& viewport) override;
        void SetTargetViewport(const math::Quad& target) override;
        math::Quad GetViewport() const override;

        void SetPosition(const math::Vector& position) override;
        math::Vector GetPosition() const override;

        math::Vector ScreenToWorld(const math::Vector& screen_pos, const math::Vector& window_size) const override;

    private:
        
        math::Vector m_offset;
        math::Quad m_viewport;
        math::Quad m_targetViewport;
        mono::MouseCameraController m_controller;

        mono::TransformSystem* m_transform_system;
        mono::EventHandler& m_event_handler;
        mono::EventToken<event::KeyDownEvent> m_key_down_token;
        
        bool m_debug_camera;
        uint32_t m_entity_id;
    };
}
