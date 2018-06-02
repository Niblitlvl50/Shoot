
#include "BeastController.h"
#include "Enemy.h"
#include "EntityProperties.h"
#include "Rendering/Sprite/ISprite.h"

using namespace game;

BeastController::BeastController(mono::EventHandler& event_handler)
{ }

void BeastController::Initialize(Enemy* enemy)
{
    enemy->m_sprite->SetAnimation(2);
    enemy->SetProperty(EntityProperties::DAMAGABLE);
}

void BeastController::doUpdate(unsigned int delta)
{
    
}
