
#include "TitleScreen.h"
#include "ZoneFlow.h"
#include "Effects/ScreenSparkles.h"

#include "RenderLayers.h"

#include "Math/Quad.h"
#include "Camera/ICamera.h"
#include "Rendering/Color.h"

#include "Events/KeyEvent.h"
#include "Events/QuitEvent.h"
#include "Events/EventFuncFwd.h"
#include "EventHandler/EventHandler.h"

#include "SystemContext.h"
#include "Particle/ParticleSystem.h"
#include "Particle/ParticleSystemDrawer.h"
#include "Rendering/Sprite/SpriteBatchDrawer.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Text/TextSystem.h"
#include "Rendering/Text/TextBatchDrawer.h"
#include "TransformSystem/TransformSystem.h"
#include "System/System.h"

#include "EntitySystem/IEntityManager.h"
#include "WorldFile.h"

using namespace game;

namespace
{
    class CheckControllerInput : public mono::IUpdatable
    {
    public:

        CheckControllerInput(TitleScreen* title_screen)
            : m_title_screen(title_screen)
        { }

        void Update(const mono::UpdateContext& update_context)
        {
            const System::ControllerState& state = System::GetController(System::ControllerId::Primary);

            const bool a_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::A);
            const bool y_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::Y);
            const bool x_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::X);

            if(a_pressed)
                m_title_screen->Continue();
            else if(y_pressed)
                m_title_screen->Quit();
            else if(x_pressed)
                m_title_screen->Remote();

            m_last_state = state;
        }

        TitleScreen* m_title_screen;
        System::ControllerState m_last_state;
    };
}

TitleScreen::TitleScreen(const ZoneCreationContext& context)
    : GameZone(context, "res/worlds/title_screen.components")
    , m_event_handler(*context.event_handler)
    , m_system_context(context.system_context)
{
    const event::KeyUpEventFunc key_callback =
        [this](const event::KeyUpEvent& event) -> mono::EventResult
    {
        if(event.key == Keycode::ENTER)
            Continue();
        else if(event.key == Keycode::Q)
            Remote();

        return mono::EventResult::PASS_ON;
    };

    m_key_token = m_event_handler.AddListener(key_callback);
}

TitleScreen::~TitleScreen()
{
    m_event_handler.RemoveListener(m_key_token);
}

void TitleScreen::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    GameZone::OnLoad(camera, renderer);

    AddUpdatable(new CheckControllerInput(this));
    mono::ParticleSystem* particle_system = m_system_context->GetSystem<mono::ParticleSystem>();
    mono::IEntityManager* entity_system = m_system_context->GetSystem<mono::IEntityManager>();
    m_sparkles = std::make_unique<ScreenSparkles>(
        particle_system, entity_system, camera->GetPosition(), camera->GetViewportSize());
}

int TitleScreen::OnUnload()
{
    GameZone::OnUnload();
    return m_exit_zone;
}

void TitleScreen::Continue()
{
    m_exit_zone = TEST_ZONE;
    m_event_handler.DispatchEvent(event::QuitEvent());
}

void TitleScreen::Remote()
{
    m_exit_zone = REMOTE_ZONE;
    m_event_handler.DispatchEvent(event::QuitEvent());
}

void TitleScreen::Quit()
{
    m_exit_zone = QUIT;
    m_event_handler.DispatchEvent(event::QuitEvent());
}
