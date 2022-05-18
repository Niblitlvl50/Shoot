
#include "PacketDeliveryGameMode.h"
#include "Hud/BigTextScreen.h"
#include "Hud/PauseScreen.h"
#include "Hud/PlayerUIElement.h"
#include "Hud/LevelTimerUIElement.h"
#include "Hud/PlayerAuxiliaryDrawer.h"
#include "Zones/ZoneFlow.h"
#include "Events/GameEvents.h"
#include "Events/GameEventFuncFwd.h"
#include "Events/PlayerEvents.h"
#include "Player/PlayerDaemon.h"
#include "Pickups/EnemyPickupSpawner.h"
#include "TriggerSystem/TriggerSystem.h"
#include "Network/ServerManager.h"
#include "RenderLayers.h"
#include "Rendering/IRenderer.h"

#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "TransformSystem/TransformSystem.h"

#include "Entity/EntityLogicSystem.h"
#include "EntitySystem/IEntityManager.h"
#include "Events/QuitEvent.h"
#include "Events/PauseEvent.h"
#include "Events/EventFuncFwd.h"
#include "EventHandler/EventHandler.h"
#include "SystemContext.h"
#include "System/Hash.h"
#include "Zone/IZone.h"
#include "Math/EasingFunctions.h"

#include "WorldFile.h"

namespace
{
    const uint32_t level_completed_hash = hash::Hash("level_completed");
}

namespace tweak_values
{
    constexpr float fade_duration_s = 1.0f;
}

using namespace game;

PacketDeliveryGameMode::PacketDeliveryGameMode()
    : m_package_spawned(false)
    , m_next_zone(game::ZoneResult::ZR_ABORTED)
    , m_package_release_callback(0)
    , m_package_entity_id(mono::INVALID_ID)
{
    const GameModeStateMachine::StateTable state_table = {
        GameModeStateMachine::MakeState(GameModeStates::FADE_IN, &PacketDeliveryGameMode::ToFadeIn, &PacketDeliveryGameMode::FadeIn, this),
        GameModeStateMachine::MakeState(GameModeStates::RUN_GAME_MODE, &PacketDeliveryGameMode::ToRunGameMode, &PacketDeliveryGameMode::RunGameMode, this),
        GameModeStateMachine::MakeState(GameModeStates::PACKAGE_DESTROYED, &PacketDeliveryGameMode::ToPackageDestroyed, &PacketDeliveryGameMode::PackageDestroyed, this),
        GameModeStateMachine::MakeState(GameModeStates::PAUSED, &PacketDeliveryGameMode::ToPaused, &PacketDeliveryGameMode::Paused, &PacketDeliveryGameMode::ExitPaused, this),
        GameModeStateMachine::MakeState(GameModeStates::FADE_OUT, &PacketDeliveryGameMode::ToFadeOut, &PacketDeliveryGameMode::FadeOut, this),
    };
    m_states.SetStateTableAndState(state_table, GameModeStates::FADE_IN);
}

PacketDeliveryGameMode::~PacketDeliveryGameMode() = default;

void PacketDeliveryGameMode::Begin(
    mono::IZone* zone,
    mono::IRenderer* renderer,
    mono::SystemContext* system_context,
    mono::EventHandler* event_handler,
    const LevelMetadata& level_metadata)
{
    m_renderer = renderer;
    m_event_handler = event_handler;
    m_trigger_system = system_context->GetSystem<game::TriggerSystem>();
    m_entity_manager = system_context->GetSystem<mono::IEntityManager>();
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();

    DamageSystem* damage_system = system_context->GetSystem<game::DamageSystem>();
    EntityLogicSystem* logic_system = system_context->GetSystem<game::EntityLogicSystem>();

    // Quit and game over events
    const GameOverFunc on_game_over = [this](const game::GameOverEvent& game_over_event) {
        m_states.TransitionTo(GameModeStates::FADE_OUT);
        m_next_zone = game::ZoneResult::ZR_GAME_OVER;
        return mono::EventResult::PASS_ON;
    };
    m_gameover_token = m_event_handler->AddListener(on_game_over);

    const event::PauseEventFunc on_pause = [this](const event::PauseEvent& pause_event) {
        const GameModeStates new_state = pause_event.pause ? GameModeStates::PAUSED : GameModeStates::RUN_GAME_MODE;
        m_states.TransitionTo(new_state);
        return mono::EventResult::PASS_ON;
    };
    m_pause_token = m_event_handler->AddListener(on_pause);

    const TriggerCallback level_completed_callback = [this](uint32_t trigger_id) {
        m_states.TransitionTo(GameModeStates::FADE_OUT);
        m_next_zone = game::ZoneResult::ZR_COMPLETED;
    };
    m_level_completed_trigger = m_trigger_system->RegisterTriggerCallback(level_completed_hash, level_completed_callback, mono::INVALID_ID);

    // Player
    game::ServerManager* server_manager = system_context->GetSystem<game::ServerManager>();

    const PlayerSpawnedCallback player_spawned_cb =
        [this](game::PlayerSpawnState spawn_state, uint32_t player_entity_id, const math::Vector& position) {
        OnSpawnPlayer(player_entity_id, position);
    };
    m_player_daemon = std::make_unique<PlayerDaemon>(
        server_manager, m_entity_manager, system_context, m_event_handler, level_metadata.player_spawn_point, player_spawned_cb);

    // Pickups
    m_pickup_spawner =
        std::make_unique<EnemyPickupSpawner>(damage_system, logic_system, m_transform_system, m_entity_manager);

    // Package
    m_package_aux_drawer = std::make_unique<PackageAuxiliaryDrawer>(m_transform_system);
    zone->AddDrawable(m_package_aux_drawer.get(), LayerId::UI);

    // UI
    m_big_text_screen = std::make_unique<BigTextScreen>(
        "Delivery failed!",
        "Your package was destroyed, press button to quit",
        mono::Color::RGBA(0.0f, 0.0f, 0.0f, 0.8f),
        mono::Color::BLACK,
        mono::Color::OFF_WHITE,
        mono::Color::GRAY);
    m_big_text_screen->Hide();

    m_pause_screen = std::make_unique<PauseScreen>();
    m_pause_screen->Hide();

    m_player_ui = std::make_unique<PlayerUIElement>(game::g_players, game::n_players, m_sprite_system, m_event_handler);

    m_level_timer = level_metadata.time_limit_s;
    m_level_has_timelimit = (m_level_timer > 0);

    m_timer_screen = std::make_unique<LevelTimerUIElement>();
    m_timer_screen->SetSeconds(m_level_timer);
    if(!m_level_has_timelimit)
        m_timer_screen->Hide();

    zone->AddUpdatableDrawable(m_big_text_screen.get(), LayerId::UI);
    zone->AddUpdatableDrawable(m_pause_screen.get(), LayerId::UI);
    zone->AddUpdatableDrawable(m_player_ui.get(), LayerId::UI);
    zone->AddUpdatableDrawable(m_timer_screen.get(), LayerId::UI);
}

int PacketDeliveryGameMode::End(mono::IZone* zone)
{
    zone->RemoveDrawable(m_package_aux_drawer.get());
    zone->RemoveUpdatableDrawable(m_big_text_screen.get());
    zone->RemoveUpdatableDrawable(m_pause_screen.get());
    zone->RemoveUpdatableDrawable(m_player_ui.get());
    zone->RemoveUpdatableDrawable(m_timer_screen.get());

    if(m_package_entity_id != mono::INVALID_ID)
        m_entity_manager->RemoveReleaseCallback(m_package_entity_id, m_package_release_callback);

    m_event_handler->RemoveListener(m_gameover_token);
    m_trigger_system->RemoveTriggerCallback(level_completed_hash, m_level_completed_trigger, mono::INVALID_ID);

    return m_next_zone;
}

void PacketDeliveryGameMode::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void PacketDeliveryGameMode::OnSpawnPlayer(uint32_t player_entity_id, const math::Vector& position)
{
    const math::Vector local_position = position; // Fix for lambda bug?

    m_sprite_system->SetSpriteEnabled(player_entity_id, false);

    const uint32_t portal_entity_id = m_entity_manager->CreateEntity("res/entities/portal_green.entity").id;
    m_transform_system->SetTransform(portal_entity_id, math::CreateMatrixWithPosition(local_position));
    m_transform_system->SetTransformState(portal_entity_id, mono::TransformState::CLIENT);

    mono::Sprite* portal_sprite = m_sprite_system->GetSprite(portal_entity_id);

    const mono::SpriteAnimationCallback set_idle_anim = [=]() {
        const mono::SpriteAnimationCallback set_end_anim = [=]() {
            const mono::SpriteAnimationCallback destroy_when_finish = [=]() {
                m_entity_manager->ReleaseEntity(portal_entity_id);
            };
            portal_sprite->SetAnimation("end", destroy_when_finish);
            SpawnPackage(local_position);
        };
        portal_sprite->SetAnimation("idle", set_end_anim);
        m_sprite_system->SetSpriteEnabled(player_entity_id, true);
        mono::IBody* player_body = m_physics_system->GetBody(player_entity_id);
        player_body->ApplyLocalImpulse(math::Vector(80.0f, 0.0f), math::ZeroVec);
    };

    portal_sprite->SetAnimation("begin", set_idle_anim);
}

void PacketDeliveryGameMode::SpawnPackage(const math::Vector& position)
{
    if(m_package_spawned)
        return;

    m_package_spawned = true;

    const mono::Entity package_entity = m_entity_manager->CreateEntity("res/entities/cardboard_box.entity");
    m_transform_system->SetTransform(package_entity.id, math::CreateMatrixWithPosition(position));
    m_transform_system->SetTransformState(package_entity.id, mono::TransformState::CLIENT);

    mono::IBody* package_body = m_physics_system->GetBody(package_entity.id);
    package_body->ApplyLocalImpulse(math::Vector(80.0f, 0.0f), math::ZeroVec);

    const mono::ReleaseCallback release_callback = [this](uint32_t entity_id) {
        m_states.TransitionTo(GameModeStates::PACKAGE_DESTROYED);
        m_package_aux_drawer->SetPackageId(-1);
    };
    m_package_release_callback = m_entity_manager->AddReleaseCallback(package_entity.id, release_callback);
    m_package_entity_id = package_entity.id;

    m_package_aux_drawer->SetPackageId(package_entity.id);

    System::Log("PacketDeliveryGameMode|Spawning package[id:%u] at position %.2f %.2f", m_package_entity_id, position.x, position.y);
}

void PacketDeliveryGameMode::ToFadeIn()
{
    m_fade_timer = 0.0f;
}
void PacketDeliveryGameMode::FadeIn(const mono::UpdateContext& update_context)
{
    const float alpha = math::EaseInCubic(m_fade_timer, tweak_values::fade_duration_s, 0.0f, 1.0f);
    m_renderer->SetScreenFadeAlpha(alpha);

    if(m_fade_timer > tweak_values::fade_duration_s)
        m_states.TransitionTo(GameModeStates::RUN_GAME_MODE);
    m_fade_timer += update_context.delta_s;
}

void PacketDeliveryGameMode::ToRunGameMode()
{ }
void PacketDeliveryGameMode::RunGameMode(const mono::UpdateContext& update_context)
{
    if(m_package_spawned)
    {
        m_level_timer -= update_context.delta_s;
        m_timer_screen->SetSeconds(m_level_timer);

        if(m_level_has_timelimit && m_level_timer < 1.0f)
        {
            // Time out, game over. Should proably be a different fail state.
            m_big_text_screen->SetSubText("Man, you were too slow!");
            m_states.TransitionTo(GameModeStates::PACKAGE_DESTROYED);
        }
    }
}

void PacketDeliveryGameMode::ToPackageDestroyed()
{
    m_next_zone = game::ZoneResult::ZR_GAME_OVER;

    m_last_state.button_state = 0;
    m_big_text_screen->SetAlpha(0.0f);
    m_big_text_screen->Show();

    m_big_text_animation_timer = 0.0f;
}
void PacketDeliveryGameMode::PackageDestroyed(const mono::UpdateContext& update_context)
{
    m_big_text_animation_timer = std::clamp(m_big_text_animation_timer + update_context.delta_s, 0.0f, 1.0f);
    const float alpha = math::EaseOutCubic(m_big_text_animation_timer, tweak_values::fade_duration_s, 0.0f, 1.0f);
    m_big_text_screen->SetAlpha(alpha);

    const System::ControllerState& state = System::GetController(System::ControllerId::Primary);
    const bool any_pressed =
        System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::FACE_ANY);
    if(any_pressed)
        m_states.TransitionTo(GameModeStates::FADE_OUT);

    m_last_state = state;
}

void PacketDeliveryGameMode::ToPaused()
{
    m_pause_screen->Show();
}
void PacketDeliveryGameMode::Paused(const mono::UpdateContext& update_context)
{

}
void PacketDeliveryGameMode::ExitPaused()
{
    m_pause_screen->Hide();
}

void PacketDeliveryGameMode::ToFadeOut()
{
    m_fade_timer = 0.0f;
}
void PacketDeliveryGameMode::FadeOut(const mono::UpdateContext& update_context)
{
    const float alpha = math::EaseOutCubic(m_fade_timer, tweak_values::fade_duration_s, 1.0f, -1.0f);
    m_renderer->SetScreenFadeAlpha(alpha);

    if(m_fade_timer > tweak_values::fade_duration_s)
        m_event_handler->DispatchEvent(event::QuitEvent());
    m_fade_timer += update_context.delta_s;
}
