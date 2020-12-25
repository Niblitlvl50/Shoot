
#include "EndScreen.h"
#include "ZoneFlow.h"

using namespace game;

EndScreen::EndScreen(const ZoneCreationContext& context)
    : GameZone(context, "res/end_screen.components")
{ }

void EndScreen::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    GameZone::OnLoad(camera, renderer);
}

int EndScreen::OnUnload()
{
    return TITLE_SCREEN;
}