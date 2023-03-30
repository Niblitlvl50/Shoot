
#include "HordeGameMode.h"

#include "Hud/LevelTimerUIElement.h"
#include "Hud/PlayerUIElement.h"
#include "Player/PlayerDaemonSystem.h"
#include "Player/PlayerAuxiliaryDrawer.h"
#include "RenderLayers.h"
#include "TransformSystem/TransformSystem.h"
#include "Weapons/WeaponSystem.h"
#include "WorldFile.h"

#include "EntitySystem/IEntityManager.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "SystemContext.h"
#include "Zone/IZone.h"

using namespace game;


HordeGameMode::HordeGameMode()
{ }

HordeGameMode::~HordeGameMode() = default;

void HordeGameMode::Begin(
    mono::IZone* zone,
    mono::IRenderer* renderer,
    mono::SystemContext* system_context,
    mono::EventHandler* event_handler,
    const LevelMetadata& level_metadata)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_entity_manager = system_context->GetSystem<mono::IEntityManager>();
    
    WeaponSystem* weapon_system = system_context->GetSystem<game::WeaponSystem>();

    // Player
    m_player_system = system_context->GetSystem<PlayerDaemonSystem>();

    const PlayerSpawnedCallback player_spawned_cb =
        [this](game::PlayerSpawnState spawn_state, uint32_t player_entity_id, const math::Vector& position) {
        OnSpawnPlayer(player_entity_id, position);
    };
    m_player_system->SpawnPlayersAt(level_metadata.player_spawn_point, player_spawned_cb);

    m_spawn_package = level_metadata.spawn_package;
    m_package_spawn_position = level_metadata.use_package_spawn_position ?
        level_metadata.package_spawn_position : level_metadata.player_spawn_point;

    m_player_ui = std::make_unique<PlayerUIElement>(game::g_players, game::n_players, weapon_system, m_sprite_system);

    m_level_timer = level_metadata.time_limit_s;
    m_level_has_timelimit = (m_level_timer > 0);

    m_timer_screen = std::make_unique<LevelTimerUIElement>();
    m_timer_screen->SetSeconds(m_level_timer);
    if(!m_level_has_timelimit)
        m_timer_screen->Hide();

    zone->AddUpdatableDrawable(m_player_ui.get(), LayerId::UI);
    zone->AddUpdatableDrawable(m_timer_screen.get(), LayerId::UI);

    // Package
    m_package_aux_drawer = std::make_unique<PackageAuxiliaryDrawer>(m_transform_system);
    zone->AddDrawable(m_package_aux_drawer.get(), LayerId::GAMEOBJECTS_UI);
}

int HordeGameMode::End(mono::IZone* zone)
{
    zone->RemoveDrawable(m_package_aux_drawer.get());
    zone->RemoveUpdatableDrawable(m_player_ui.get());
    zone->RemoveUpdatableDrawable(m_timer_screen.get());

    if(m_package_entity_id != mono::INVALID_ID)
        m_entity_manager->RemoveReleaseCallback(m_package_entity_id, m_package_release_callback);

    return 0;
}

void HordeGameMode::Update(const mono::UpdateContext& update_context)
{

}

void HordeGameMode::OnSpawnPlayer(uint32_t player_entity_id, const math::Vector& position)
{
    const uint32_t portal_entity_id = m_entity_manager->CreateEntity("res/entities/portal_green.entity").id;
    m_transform_system->SetTransform(portal_entity_id, math::CreateMatrixWithPosition(position));
    m_transform_system->SetTransformState(portal_entity_id, mono::TransformState::CLIENT);

    m_sprite_system->SetSpriteEnabled(player_entity_id, false);

    const mono::SpriteAnimationCallback on_begin_finished = [this, player_entity_id](uint32_t sprite_id) {
        m_sprite_system->SetSpriteEnabled(player_entity_id, true);
    };

    const mono::SpriteAnimationCallback on_idle_finished = [this](uint32_t sprite_id) {
        SpawnPackage(m_package_spawn_position);
    };

    const mono::SpriteAnimationCallback destroy_when_finish = [this, portal_entity_id](uint32_t sprite_id) {
        m_entity_manager->ReleaseEntity(portal_entity_id);
    };

    const std::vector<mono::SpriteAnimNode> anim_sequence = {
        { "begin", on_begin_finished },
        { "idle", on_idle_finished },
        { "end", destroy_when_finish }
    };
    m_sprite_system->RunSpriteAnimSequence(portal_entity_id, anim_sequence);
}

void HordeGameMode::SpawnPackage(const math::Vector& position)
{
    if(!m_spawn_package)
        return;

    if(m_package_spawned)
        return;

    m_package_spawned = true;

    m_package_entity_id = m_player_system->SpawnPackageAt(position);

    const mono::ReleaseCallback release_callback = [](uint32_t entity_id) {
        //m_states.TransitionTo(GameModeStates::PACKAGE_DESTROYED);
        //m_big_text_screen->SetSubText("Your package was destroyed.");
    };
    m_package_release_callback = m_entity_manager->AddReleaseCallback(m_package_entity_id, release_callback);

    System::Log("PacketDeliveryGameMode|Spawning package[id:%u] at position %.2f %.2f", m_package_entity_id, position.x, position.y);
}
