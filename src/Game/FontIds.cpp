
#include "FontIds.h"
#include "Rendering/Text/TextFunctions.h"

bool game::LoadFonts()
{
    mono::LoadFont(FontId::PIXELETTE_TINY,   "res/fonts/pixelette.ttf", 10.0f, 1.0f / 25.0f);
    mono::LoadFont(FontId::PIXELETTE_SMALL,  "res/fonts/pixelette.ttf", 10.0f, 1.0f / 10.0f);
    mono::LoadFont(FontId::PIXELETTE_MEDIUM, "res/fonts/pixelette.ttf", 10.0f, 1.0f / 5.0f);
    mono::LoadFont(FontId::PIXELETTE_LARGE,  "res/fonts/pixelette.ttf", 10.0f, 1.0f / 3.0f);
    mono::LoadFont(FontId::PIXELETTE_MEGA,   "res/fonts/pixelette.ttf", 10.0f, 1.0f / 1.5f);

    mono::LoadFont(FontId::RUSSOONE_TINY,   "res/fonts/russoone-regular.ttf", 50.0f, 1.0f / 100.0f);
    mono::LoadFont(FontId::RUSSOONE_SMALL,  "res/fonts/russoone-regular.ttf", 72.0f, 1.0f / 60.0f);
    mono::LoadFont(FontId::RUSSOONE_MEDIUM, "res/fonts/russoone-regular.ttf", 72.0f, 1.0f / 50.0f);
    mono::LoadFont(FontId::RUSSOONE_LARGE,  "res/fonts/russoone-regular.ttf", 72.0f, 1.0f / 45.0f);
    mono::LoadFont(FontId::RUSSOONE_MEGA,   "res/fonts/russoone-regular.ttf", 72.0f, 1.0f / 20.0f);

    mono::LoadFont(FontId::MITR_TINY,   "res/fonts/mitr-regular.ttf", 72.0f, 1.0f / 80.0f);
    mono::LoadFont(FontId::MITR_SMALL,  "res/fonts/mitr-regular.ttf", 72.0f, 1.0f / 60.0f);
    mono::LoadFont(FontId::MITR_MEDIUM, "res/fonts/mitr-regular.ttf", 72.0f, 1.0f / 50.0f);
    mono::LoadFont(FontId::MITR_LARGE,  "res/fonts/mitr-regular.ttf", 72.0f, 1.0f / 45.0f);
    mono::LoadFont(FontId::MITR_MEGA,   "res/fonts/mitr-regular.ttf", 72.0f, 1.0f / 20.0f);

    mono::LoadFont(FontId::BLACK_HAN_SANS_TINY,     "res/fonts/blackhansans-regular.ttf", 72.0f, 1.0f / 25.0f);
    mono::LoadFont(FontId::BLACK_HAN_SANS_SMALL,    "res/fonts/blackhansans-regular.ttf", 72.0f, 1.0f / 10.0f);
    mono::LoadFont(FontId::BLACK_HAN_SANS_MEDIUM,   "res/fonts/blackhansans-regular.ttf", 72.0f, 1.0f / 5.0f);
    mono::LoadFont(FontId::BLACK_HAN_SANS_LARGE,    "res/fonts/blackhansans-regular.ttf", 72.0f, 1.0f / 3.0f);
    mono::LoadFont(FontId::BLACK_HAN_SANS_MEGA,     "res/fonts/blackhansans-regular.ttf", 72.0f, 1.0f / 1.5f);

    return true;
}
