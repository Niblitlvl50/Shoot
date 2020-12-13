
#include "FontIds.h"
#include "Rendering/Text/TextFunctions.h"

bool shared::LoadFonts()
{
    mono::LoadFont(shared::FontId::PIXELETTE_TINY,   "res/fonts/pixelette.ttf", 10.0f, 1.0f / 25.0f);
    mono::LoadFont(shared::FontId::PIXELETTE_SMALL,  "res/fonts/pixelette.ttf", 10.0f, 1.0f / 10.0f);
    mono::LoadFont(shared::FontId::PIXELETTE_MEDIUM, "res/fonts/pixelette.ttf", 10.0f, 1.0f / 5.0f);
    mono::LoadFont(shared::FontId::PIXELETTE_LARGE,  "res/fonts/pixelette.ttf", 10.0f, 1.0f / 3.0f);
    mono::LoadFont(shared::FontId::PIXELETTE_MEGA,   "res/fonts/pixelette.ttf", 10.0f, 1.0f / 1.5f);

    mono::LoadFont(shared::FontId::RUSSOONE_TINY,   "res/fonts/russoone-regular.ttf", 10.0f, 1.0f); // / 25.0f);
    mono::LoadFont(shared::FontId::RUSSOONE_SMALL,  "res/fonts/russoone-regular.ttf", 20.0f, 1.0f); // / 10.0f);
    mono::LoadFont(shared::FontId::RUSSOONE_MEDIUM, "res/fonts/russoone-regular.ttf", 30.0f, 1.0f); // / 5.0f);
    mono::LoadFont(shared::FontId::RUSSOONE_LARGE,  "res/fonts/russoone-regular.ttf", 40.0f, 1.0f); // / 3.0f);
    mono::LoadFont(shared::FontId::RUSSOONE_MEGA,   "res/fonts/russoone-regular.ttf", 50.0f, 1.0f); // / 1.5f);

    mono::LoadFont(shared::FontId::MONOTON_TINY,   "res/fonts/monoton-regular.ttf", 72.0f, 1.0f / 25.0f);
    mono::LoadFont(shared::FontId::MONOTON_SMALL,  "res/fonts/monoton-regular.ttf", 72.0f, 1.0f / 10.0f);
    mono::LoadFont(shared::FontId::MONOTON_MEDIUM, "res/fonts/monoton-regular.ttf", 72.0f, 1.0f / 5.0f);
    mono::LoadFont(shared::FontId::MONOTON_LARGE,  "res/fonts/monoton-regular.ttf", 72.0f, 1.0f / 3.0f);
    mono::LoadFont(shared::FontId::MONOTON_MEGA,   "res/fonts/monoton-regular.ttf", 72.0f, 1.0f / 1.5f);

    return true;
}
