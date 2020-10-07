
#include "RemoteZone.h"
#include "ZoneFlow.h"
#include "Factories.h"
#include "SpriteResources.h"
#include "RenderLayers.h"

#include "Hud/Debug/ConsoleDrawer.h"
#include "Hud/Overlay.h"
#include "Hud/Debug/NetworkStatusDrawer.h"
#include "Hud/Debug/FPSElement.h"

#include "WorldFile.h"
#include "Navigation/NavmeshFactory.h"

#include "Network/NetworkMessage.h"
#include "Network/ClientReplicator.h"
#include "Network/ClientManager.h"

#include "Camera/ICamera.h"
#include "UpdateTasks/GameCamera.h"
#include "Rendering/Sprite/SpriteBatchDrawer.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"

#include "EventHandler/EventHandler.h"
//#include "Entity/IEntity.h"
#include "Math/Vector.h"
#include "Math/Quad.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "PositionPredictionSystem.h"

#include "Player/PlayerDaemon.h"
#include "GameMode/CaptureTheFlagHud.h"

using namespace game;

RemoteZone::RemoteZone(const ZoneCreationContext& context)
    : m_system_context(context.system_context)
    , m_event_handler(*context.event_handler)
    , m_game_config(*context.game_config)
{
    using namespace std::placeholders;

    const std::function<mono::EventResult (const TextMessage&)> text_func = std::bind(&RemoteZone::HandleText, this, _1);
    const std::function<mono::EventResult (const SpawnMessage&)> spawn_func = std::bind(&RemoteZone::HandleSpawnMessage, this, _1);
    const std::function<mono::EventResult (const SpriteMessage&)> sprite_func = std::bind(&RemoteZone::HandleSpriteMessage, this, _1);

    m_text_token = m_event_handler.AddListener(text_func);
    m_spawn_token = m_event_handler.AddListener(spawn_func);
    m_sprite_token = m_event_handler.AddListener(sprite_func);

    m_ctf_score.red = 0;
    m_ctf_score.blue = 0;
}

RemoteZone::~RemoteZone()
{
    m_event_handler.RemoveListener(m_text_token);
    m_event_handler.RemoveListener(m_spawn_token);
    m_event_handler.RemoveListener(m_sprite_token);
}

void RemoteZone::OnLoad(mono::ICamera* camera)
{
    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();

    camera->SetViewport(math::Quad(0, 0, 22, 14));
    m_game_camera = std::make_unique<GameCamera>(camera, transform_system, m_event_handler);

    m_client_manager = std::make_unique<ClientManager>(&m_event_handler, &m_game_config);
    m_player_daemon = std::make_unique<ClientPlayerDaemon>(m_game_camera.get(), m_event_handler);

    const PositionPredictionSystem* prediction_system =
        m_system_context->CreateSystem<PositionPredictionSystem>(500, m_client_manager.get(), transform_system, &m_event_handler);

    AddUpdatable(m_game_camera.get());
    AddUpdatable(m_client_manager.get());
    AddUpdatable(new ClientReplicator(camera, m_client_manager.get()));

    AddDrawable(new mono::SpriteBatchDrawer(transform_system, sprite_system), LayerId::GAMEOBJECTS);
    AddDrawable(new PredictionSystemDebugDrawer(prediction_system), LayerId::GAMEOBJECTS_DEBUG);
    AddDrawable(new CaptureTheFlagHud(m_ctf_score), LayerId::UI);

    m_console_drawer = std::make_unique<ConsoleDrawer>();
    AddDrawable(m_console_drawer.get(), LayerId::UI);

    auto hud_overlay = new UIOverlayDrawer();
    hud_overlay->AddChild(new NetworkStatusDrawer(math::Vector(2.0f, 190.0f), m_client_manager.get()));
    hud_overlay->AddChild(new FPSElement(math::Vector(2.0f, 2.0f), mono::Color::BLACK));
    AddEntity(hud_overlay, LayerId::UI);
}

int RemoteZone::OnUnload()
{
    RemoveUpdatable(m_game_camera.get());
    RemoveUpdatable(m_client_manager.get());
    RemoveDrawable(m_console_drawer.get());
    return 0;
}

mono::EventResult RemoteZone::HandleText(const TextMessage& text_message)
{
    m_console_drawer->AddText(text_message.text, 1500);
    return mono::EventResult::HANDLED;
}

mono::EventResult RemoteZone::HandleSpawnMessage(const SpawnMessage& spawn_message)
{
    mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();

    if(!spawn_message.spawn)
        sprite_system->ReleaseSprite(spawn_message.entity_id);

    return mono::EventResult::HANDLED;
}

mono::EventResult RemoteZone::HandleSpriteMessage(const SpriteMessage& sprite_message)
{
    mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();
    const bool is_allocated = sprite_system->IsAllocated(sprite_message.entity_id);
    if(!is_allocated)
    {
        mono::SpriteComponents sprite_data;
        sprite_data.sprite_file = game::SpriteHashToString(sprite_message.filename_hash);
        sprite_system->AllocateSprite(sprite_message.entity_id, sprite_data);
    }

    mono::Sprite* sprite = sprite_system->GetSprite(sprite_message.entity_id);

    sprite->SetShade(mono::Color::ToRGBA(sprite_message.hex_color));
    sprite->SetAnimation(sprite_message.animation_id);
    sprite->SetHorizontalDirection(mono::HorizontalDirection(sprite_message.horizontal_direction));
    sprite->SetVerticalDirection(mono::VerticalDirection(sprite_message.vertical_direction));

    return mono::EventResult::HANDLED;
}
