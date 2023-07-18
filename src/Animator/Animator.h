
#pragma once

#include "MonoFwd.h"
#include "System/System.h"
#include "Zone/ZoneBase.h"
#include "EventHandler/EventToken.h"
#include "Events/EventFwd.h"
#include "UIContext.h"

#include "Rendering/RenderFwd.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "ImGuiImpl/ImGuiInputHandler.h"

#include <memory>

namespace animator
{
    class Animator : public mono::ZoneBase
    {
    public:

        Animator(
            mono::TransformSystem* transform_system,
            mono::SpriteSystem* sprite_system,
            mono::RenderSystem* render_system,
            mono::EntitySystem* entity_system,
            mono::EventHandler* event_handler,
            float pixels_per_meter,
            const char* sprite_file);
        ~Animator();

        void OnLoad(mono::ICamera* camera, mono::IRenderer* renderer) override;
        int OnUnload() override;

        void OpenSpriteFile(const std::string& sprite_file);

        mono::EventResult OnKeyDownUp(const event::KeyDownEvent& event);
        mono::EventResult OnMouseWheel(const event::MouseWheelEvent& event);
        mono::EventResult OnMultiGesture(const event::MultiGestureEvent& event);

        void OnAddAnimation();
        void OnDeleteAnimation();
        void SetAnimation(int animation_id);

        void OnNameAnimation(const char* new_name);
        void OnLoopToggle(bool state);
        void SetFrameDuration(int new_frame_duration);
        
        void OnAddFrame();
        void OnDeleteFrame(int id);
        void SetActiveFrame(int frame);
        void SetAnimationFrame(int animation_frame_index, int new_frame);
        void SetFrameOffset(const math::Vector& frame_offset_pixels);

        void SaveSprite();
        void SetSpeed(float new_speed);
        void TogglePlaying();
        void ToggleOffsetMode();

    private:

        void Zoom(float multiplier);

        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        mono::RenderSystem* m_render_system;
        mono::EntitySystem* m_entity_system;
        mono::EventHandler* m_event_handler;
        const float m_pixels_per_meter;

        mono::EventToken<event::KeyDownEvent> m_key_down_token;
        mono::EventToken<event::MouseWheelEvent> m_mouse_wheel_token;
        mono::EventToken<event::MultiGestureEvent> m_multi_gesture_token;

        std::unique_ptr<ImGuiInputHandler> m_input_handler;

        UIContext m_context;

        mono::ICamera* m_camera;
        mono::ITexturePtr m_tools_texture;

        mono::Entity* m_entity;
        mono::SpriteData* m_sprite_data;
        mono::ISprite* m_sprite;
        mono::SpriteBatchDrawer* m_sprite_batch_drawer;
        class SpriteOffsetDrawer* m_Sprite_offset_drawer;
    };
}
