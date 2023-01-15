
#include "UISystemDrawer.h"
#include "UISystem.h"

#include "Math/Matrix.h"
#include "Math/Quad.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Rendering/RenderBuffer/IRenderBuffer.h"
#include "Rendering/RenderBuffer/BufferFactory.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteBufferFactory.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteProperties.h"

#include "TransformSystem/TransformSystem.h"
#include "EntitySystem/Entity.h"

using namespace game;

UISystemDrawer::UISystemDrawer(const UISystem* ui_system, mono::TransformSystem* transform_system)
    : m_ui_system(ui_system)
    , m_transform_system(transform_system)
{
    m_sprite = mono::RenderSystem::GetSpriteFactory()->CreateSprite("res/sprites/ui_item_selection.sprite");
    m_sprite_buffer = mono::BuildSpriteDrawBuffers(m_sprite->GetSpriteData());

    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3
    };
    m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, 6, indices);
}

void UISystemDrawer::Draw(mono::IRenderer& renderer) const
{
    if(!m_ui_system->IsEnabled())
        return;

    const uint32_t active_item = m_ui_system->GetActiveEntityItem();
    if(active_item != mono::INVALID_ID)
    {
        const math::Quad world_bb = math::ResizeQuad(m_transform_system->GetWorldBoundingBox(active_item), 0.1f);

        {
            m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

            const math::Matrix transform = math::CreateMatrixWithPosition(math::LeftCenter(world_bb));
            const auto transform_scope = mono::MakeTransformScope(transform, &renderer);
            renderer.DrawSprite(m_sprite.get(), &m_sprite_buffer, m_indices.get());
        }

        {
            m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

            const math::Matrix transform = math::CreateMatrixWithPosition(math::RightCenter(world_bb));
            const auto transform_scope = mono::MakeTransformScope(transform, &renderer);
            renderer.DrawSprite(m_sprite.get(), &m_sprite_buffer, m_indices.get());
        }
    }

    const bool draw_cursor = m_ui_system->DrawCursor();
    if(draw_cursor)
    {
        const math::Vector& cursor_target_position = m_ui_system->GetCursorTargetPosition();
        renderer.DrawPoints({cursor_target_position}, mono::Color::RED, 15.0f);
    }
}

math::Quad UISystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
