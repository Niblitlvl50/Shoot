
#include "PlayerFamiliarLogic.h"

#include "EventHandler/EventHandler.h"
#include "Events/EventFuncFwd.h"
#include "Events/MouseEvent.h"

#include "SystemContext.h"
#include "System/System.h"

#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "TransformSystem/TransformSystem.h"

#include "Math/CriticalDampedSpring.h"

namespace tweak_values
{
    constexpr float idle_threshold_s = 2.0f;
    constexpr float move_halflife = 0.3f;
}

using namespace game;

PlayerFamiliarLogic::PlayerFamiliarLogic(
    uint32_t entity_id, mono::EventHandler* event_handler, mono::SystemContext* system_context)
    : m_entity_id(entity_id)
    , m_event_handler(event_handler)
    , m_idle_timer(tweak_values::idle_threshold_s)
{
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();

    m_sprite_system->SetSpriteEnabled(m_entity_id, false);

    using namespace std::placeholders;
    const event::MouseMotionEventFunc mouse_motion_func = std::bind(&PlayerFamiliarLogic::OnMouseMotion, this, _1);
    m_mouse_motion_token = m_event_handler->AddListener(mouse_motion_func);
}

PlayerFamiliarLogic::~PlayerFamiliarLogic()
{
    m_event_handler->RemoveListener(m_mouse_motion_token);
}

void PlayerFamiliarLogic::Update(const mono::UpdateContext& update_context)
{
    m_idle_timer += update_context.delta_s;

    const bool show_sprite = (m_idle_timer < tweak_values::idle_threshold_s);
    m_sprite_system->SetSpriteEnabled(m_entity_id, show_sprite);

    if(show_sprite)
    {
        math::Vector current_position = m_transform_system->GetWorldPosition(m_entity_id);

        math::critical_spring_damper(
            current_position,
            m_move_velocity,
            m_target_position,
            math::ZeroVec,
            tweak_values::move_halflife,
            update_context.delta_s);

        const bool face_left = (m_move_velocity.x < 0.0f);
        mono::Sprite* sprite = m_sprite_system->GetSprite(m_entity_id);

        if(face_left)
            sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
        else
            sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

        const math::Matrix new_transform = math::CreateMatrixWithPosition(current_position);
        m_transform_system->SetTransform(m_entity_id, new_transform);
    }
}

mono::EventResult PlayerFamiliarLogic::OnMouseMotion(const event::MouseMotionEvent& event)
{
    m_idle_timer = 0.0f;
    m_target_position = math::Vector(event.world_x, event.world_y);
    return mono::EventResult::PASS_ON;
}
