
#pragma once

#include "Camera/ICamera.h"
#include "Camera/CameraController.h"
#include "MonoFwd.h"
#include "Math/Quad.h"

#include "EventHandler/EventToken.h"
#include "Events/EventFwd.h"

namespace game
{
    class Camera : public mono::ICamera
    {
    public:

        Camera(int width, int height, int window_width, int window_height, mono::EventHandler& event_handler);
        ~Camera();

        bool OnKeyDown(const event::KeyDownEvent& event);
        
        void doUpdate(unsigned int delta) override;
        
        void Follow(const mono::IEntityPtr& entity, const math::Vector& offset) override;
        void Unfollow() override;
        math::Quad GetViewport() const override;
        math::Vector GetPosition() const override;
        void SetViewport(const math::Quad& viewport) override;
        void SetTargetViewport(const math::Quad& target) override;
        void SetPosition(const math::Vector& position) override;
        math::Vector ScreenToWorld(const math::Vector& screen_pos, const math::Vector& window_size) const override;

    private:
        
        math::Vector m_offset;
        math::Quad m_viewport;
        math::Quad m_targetViewport;
        mono::MouseCameraController m_controller;

        mono::EventHandler& m_event_handler;
        mono::EventToken<event::KeyDownEvent> m_key_down_token;
        
        bool m_debug_camera;

        mono::IEntityPtr m_entity;
    };
}
