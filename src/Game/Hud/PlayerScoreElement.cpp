
#include "PlayerScoreElement.h"
#include "AIKnowledge.h"
#include "FontIds.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"

#include <cstdio>

using namespace game;

PlayerScoreElement::PlayerScoreElement(const PlayerInfo& player_info, const math::Vector& position)
    : m_player_info(player_info)
{
    m_position = position;
}

PlayerScoreElement::~PlayerScoreElement()
{ }

void PlayerScoreElement::Draw(mono::IRenderer& renderer) const
{
    char score_buffer[256] = { 0 };
    std::sprintf(score_buffer, "score %010d", m_player_info.score);
    renderer.DrawText(FontId::PIXELETTE_MEGA, score_buffer, math::ZeroVec, false, mono::Color::BLUE);
}

void PlayerScoreElement::Update(const mono::UpdateContext& update_context)
{

}
