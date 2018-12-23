
#include "BatController.h"
#include "Enemy.h"

using namespace game;

BatController::BatController(mono::EventHandler& event_handler)
{

}

void BatController::Initialize(Enemy* enemy)
{
    m_enemy = enemy;
}

void BatController::doUpdate(unsigned int delta)
{

}
