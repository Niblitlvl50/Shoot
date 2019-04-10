
#include "ConsoleDrawer.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "System/System.h"
#include "Util/Algorithm.h"

using namespace game;

void ConsoleDrawer::AddText(const std::string& text)
{
    TextItem item;
    item.text = text;
    item.life = System::GetMilliseconds();

    m_text_items.push_back(item);
}

void ConsoleDrawer::doDraw(mono::IRenderer& renderer) const
{
    const math::Matrix& projection = math::Ortho(0.0f, 20.0f, 0.0f, 16.0f, 0.0f, 10.0f);
    renderer.PushNewProjection(projection);
    renderer.PushNewTransform(math::Matrix());

    math::Vector current_pos;
    mono::Color::RGBA color(1, 0, 0);

    for(const TextItem& item : m_text_items)
    {
        renderer.DrawText(0, item.text.c_str(), current_pos, false, color);
        current_pos.y += 1.0f;
    }

    const auto check_for_removal = [](const TextItem& item) {
        return (System::GetMilliseconds() - item.life) > 1500;
    };

    mono::remove_if(m_text_items, check_for_removal);
}

math::Quad ConsoleDrawer::BoundingBox() const
{
    return math::InfQuad;
}
