
#include "TitleScreen.h"
#include "ZoneFlow.h"
#include "Factories.h"
#include "Effects/ScreenSparkles.h"
#include "GameDebugDrawer.h"

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
#include "Rendering/Text/TextSystem.h"
#include "Rendering/Text/TextBatchDrawer.h"
#include "TransformSystem/TransformSystem.h"
#include "System/System.h"

#include "Entity/IEntityManager.h"
#include "WorldFile.h"

using namespace game;

#define IS_TRIGGERED(variable) (!m_last_state.variable && state.variable)
#define HAS_CHANGED(variable) (m_last_state.variable != state.variable)

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

            const bool a_pressed = IS_TRIGGERED(a) && HAS_CHANGED(a);
            const bool y_pressed = IS_TRIGGERED(y) && HAS_CHANGED(y);
            const bool x_pressed = IS_TRIGGERED(x) && HAS_CHANGED(x);

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
    : m_event_handler(*context.event_handler)
    , m_system_context(context.system_context)
{
    using namespace std::placeholders;
    const event::KeyUpEventFunc& key_callback = std::bind(&TitleScreen::OnKeyUp, this, _1);
    m_key_token = m_event_handler.AddListener(key_callback);
}

TitleScreen::~TitleScreen()
{
    m_event_handler.RemoveListener(m_key_token);
}

mono::EventResult TitleScreen::OnKeyUp(const event::KeyUpEvent& event)
{
    if(event.key == Keycode::ENTER)
        Continue();
    else if(event.key == Keycode::R)
        Remote();

    return mono::EventResult::PASS_ON;
}

void TitleScreen::OnLoad(mono::ICamera* camera)
{
    const shared::LevelData leveldata = shared::ReadWorldComponentObjects("res/title_screen.components", g_entity_manager);
    const math::Quad viewport(
        leveldata.metadata.camera_position, leveldata.metadata.camera_position + leveldata.metadata.camera_size);

    camera->SetViewport(viewport);
    m_loaded_entities = leveldata.loaded_entities;

    mono::ParticleSystem* particle_system = m_system_context->GetSystem<mono::ParticleSystem>();
    mono::TextSystem* text_system = m_system_context->GetSystem<mono::TextSystem>();
    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();

    AddUpdatable(new CheckControllerInput(this));

    AddDrawable(new mono::SpriteBatchDrawer(m_system_context), LayerId::GAMEOBJECTS);
    AddDrawable(new mono::TextBatchDrawer(text_system, transform_system), LayerId::GAMEOBJECTS_DEBUG);
    AddDrawable(new mono::ParticleSystemDrawer(particle_system), LayerId::GAMEOBJECTS);
    AddDrawable(new GameDebugDrawer(), LayerId::GAMEOBJECTS_DEBUG);

    m_sparkles = std::make_unique<ScreenSparkles>(particle_system, viewport);
}

int TitleScreen::OnUnload()
{
    for(uint32_t entity_id : m_loaded_entities)
        g_entity_manager->ReleaseEntity(entity_id);

    g_entity_manager->Sync();
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
