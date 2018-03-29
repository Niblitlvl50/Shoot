
#include "Overlay.h"
#include "Math/Quad.h"
#include "Rendering/IRenderer.h"

using namespace game;

UIOverlayDrawer::UIOverlayDrawer()
{
    m_projection = math::Ortho(0.0f, 300.0f, 0.0f, 200.0f, -10.0f, 10.0f);    
}

UIOverlayDrawer::~UIOverlayDrawer()
{

}

void UIOverlayDrawer::doDraw(mono::IRenderer& renderer) const
{
    constexpr math::Matrix identity;
    renderer.PushNewTransform(identity);
    renderer.PushNewProjection(m_projection);

    for(auto& element : m_ui_elements)
        element->Draw(renderer);
}

math::Quad UIOverlayDrawer::BoundingBox() const
{
    return math::InfQuad;
}

int UIOverlayDrawer::AddElement(std::unique_ptr<UIElement> element)
{
    m_ui_elements.push_back(std::move(element));
    return m_ui_elements.size() -1;
}
