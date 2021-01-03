
#include "RemoteZone.h"
#include "ZoneFlow.h"
#include "Factories.h"
#include "SpriteResources.h"
#include "RenderLayers.h"
#include "GameDebug.h"

#include "Hud/Debug/ConsoleDrawer.h"
#include "Hud/Overlay.h"
#include "Hud/Debug/NetworkStatusDrawer.h"
#include "Hud/Debug/FPSElement.h"

#include "Network/NetworkMessage.h"
#include "Network/ClientReplicator.h"
#include "Network/ClientManager.h"

#include "Camera/ICamera.h"
#include "EventHandler/EventHandler.h"
#include "Math/Vector.h"
#include "Rendering/Sprite/SpriteBatchDrawer.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"

#include "GameCamera/CameraSystem.h"
#include "PositionPredictionSystem.h"
#include "Player/ClientPlayerDaemon.h"
#include "World/StaticBackground.h"

#include "ImGuiImpl/ImGuiInputHandler.h"

using namespace game;

RemoteZone::RemoteZone(const ZoneCreationContext& context)
    : m_system_context(context.system_context)
    , m_event_handler(context.event_handler)
    , m_game_config(*context.game_config)
{
    using namespace std::placeholders;

    const std::function<mono::EventResult (const LevelMetadataMessage&)> metadata_func = std::bind(&RemoteZone::HandleLevelMetadata, this, _1);
    const std::function<mono::EventResult (const TextMessage&)> text_func = std::bind(&RemoteZone::HandleText, this, _1);
    const std::function<mono::EventResult (const SpawnMessage&)> spawn_func = std::bind(&RemoteZone::HandleSpawnMessage, this, _1);
    const std::function<mono::EventResult (const SpriteMessage&)> sprite_func = std::bind(&RemoteZone::HandleSpriteMessage, this, _1);

    m_metadata_token = m_event_handler->AddListener(metadata_func);
    m_text_token = m_event_handler->AddListener(text_func);
    m_spawn_token = m_event_handler->AddListener(spawn_func);
    m_sprite_token = m_event_handler->AddListener(sprite_func);
}

RemoteZone::~RemoteZone()
{
    m_event_handler->RemoveListener(m_metadata_token);
    m_event_handler->RemoveListener(m_text_token);
    m_event_handler->RemoveListener(m_spawn_token);
    m_event_handler->RemoveListener(m_sprite_token);
}

void RemoteZone::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    m_camera = camera;

    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    m_sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();
    CameraSystem* camera_system = m_system_context->GetSystem<CameraSystem>();
    ClientManager* client_manager = m_system_context->GetSystem<ClientManager>();
    client_manager->StartClient();

    m_player_daemon = std::make_unique<ClientPlayerDaemon>(camera_system, m_event_handler);

    const PositionPredictionSystem* prediction_system =
        m_system_context->CreateSystem<PositionPredictionSystem>(500, client_manager, transform_system, m_event_handler);

    m_debug_input = std::make_unique<ImGuiInputHandler>(*m_event_handler);
    m_console_drawer = std::make_unique<ConsoleDrawer>();

    AddUpdatable(new ClientReplicator(camera, client_manager));
    AddDrawable(new mono::SpriteBatchDrawer(transform_system, m_sprite_system), LayerId::GAMEOBJECTS);
    AddDrawable(new PredictionSystemDebugDrawer(prediction_system), LayerId::GAMEOBJECTS_DEBUG);
    AddDrawable(m_console_drawer.get(), LayerId::UI);

    AddUpdatable(new DebugUpdater(m_event_handler));

    auto hud_overlay = new UIOverlayDrawer();
    hud_overlay->AddChild(new NetworkStatusDrawer(math::Vector(2.0f, 190.0f), client_manager));
    hud_overlay->AddChild(new FPSElement(math::Vector(2.0f, 2.0f), mono::Color::BLACK));
    AddEntity(hud_overlay, LayerId::UI);
}

int RemoteZone::OnUnload()
{
    ClientManager* client_manager = m_system_context->GetSystem<ClientManager>();
    client_manager->Disconnect();

    RemoveDrawable(m_console_drawer.get());
    return 0;
}

mono::EventResult RemoteZone::HandleLevelMetadata(const LevelMetadataMessage& metadata_message)
{
    m_camera->SetPosition(metadata_message.camera_position);
    m_camera->SetViewportSize(metadata_message.camera_size);

    //const shared::LevelData level_data = shared::ReadWorldComponentObjects(m_world_file, entity_system, nullptr);
    //const char* world_filename = WorldHashToString(metadata_message.world_file_hash);

    const char* background_texture_filename = game::HashToFilename(metadata_message.background_texture_hash);
    if(background_texture_filename)
        AddDrawable(new StaticBackground(background_texture_filename), LayerId::BACKGROUND);

    return mono::EventResult::HANDLED;
}

mono::EventResult RemoteZone::HandleText(const TextMessage& text_message)
{
    m_console_drawer->AddText(text_message.text, 1500);
    return mono::EventResult::HANDLED;
}

mono::EventResult RemoteZone::HandleSpawnMessage(const SpawnMessage& spawn_message)
{
    if(!spawn_message.spawn)
    {
        mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();
        sprite_system->ReleaseSprite(spawn_message.entity_id);
    }

    return mono::EventResult::HANDLED;
}

mono::EventResult RemoteZone::HandleSpriteMessage(const SpriteMessage& sprite_message)
{
    const bool is_allocated = m_sprite_system->IsAllocated(sprite_message.entity_id);
    if(!is_allocated)
    {
        mono::SpriteComponents sprite_data;
        sprite_data.sprite_file = game::HashToFilename(sprite_message.filename_hash);
        m_sprite_system->AllocateSprite(sprite_message.entity_id, sprite_data);
    }

    mono::Sprite* sprite = m_sprite_system->GetSprite(sprite_message.entity_id);

    sprite->SetShade(mono::Color::ToRGBA(sprite_message.hex_color));
    sprite->SetAnimation(sprite_message.animation_id);
    sprite->SetHorizontalDirection(mono::HorizontalDirection(sprite_message.horizontal_direction));
    sprite->SetVerticalDirection(mono::VerticalDirection(sprite_message.vertical_direction));

    const math::Vector shadow_offset(sprite_message.shadow_offset_x, sprite_message.shadow_offset_y);
    sprite->SetShadowOffset(shadow_offset);
    sprite->SetShadowSize(sprite_message.shadow_size);
    sprite->SetProperties(sprite_message.properties);

    m_sprite_system->SetSpriteLayer(sprite_message.entity_id, sprite_message.layer);

    return mono::EventResult::HANDLED;
}
