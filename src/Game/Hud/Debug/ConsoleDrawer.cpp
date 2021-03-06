
#include "ConsoleDrawer.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "System/System.h"
#include "Util/Algorithm.h"

using namespace game;

void ConsoleDrawer::AddText(const std::string& text, uint32_t life_time_ms)
{
    TextItem item;
    item.text = text;
    item.life = System::GetMilliseconds() + life_time_ms;

    m_text_items.push_back(item);
}

void ConsoleDrawer::Draw(mono::IRenderer& renderer) const
{
    const math::Matrix& projection = math::Ortho(0.0f, 20.0f, 0.0f, 16.0f, 0.0f, 10.0f);
    const mono::ScopedTransform projection_scope = mono::MakeProjectionScope(projection, &renderer);
    const mono::ScopedTransform view_scope = mono::MakeViewTransformScope(math::Matrix(), &renderer);
    const mono::ScopedTransform transform_scope = mono::MakeTransformScope(math::Matrix(), &renderer);

    math::Vector current_pos;

    for(const TextItem& item : m_text_items)
    {
        const math::Matrix world_transform = math::CreateMatrixWithPosition(current_pos);
        const auto scope = mono::MakeTransformScope(world_transform, &renderer);

        renderer.RenderText(0, item.text.c_str(), mono::Color::RED, mono::FontCentering::DEFAULT_CENTER);
        current_pos.y += 1.0f;
    }

    const auto check_for_removal = [](const TextItem& item) {
        return System::GetMilliseconds() > item.life;
    };

    mono::remove_if(m_text_items, check_for_removal);
}

math::Quad ConsoleDrawer::BoundingBox() const
{
    return math::InfQuad;
}
