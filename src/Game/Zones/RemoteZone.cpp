
#include "RemoteZone.h"
#include "ZoneFlow.h"
#include "Factories.h"
#include "SpriteResources.h"
#include "RenderLayers.h"
#include "GameDebug.h"

#include "Hud/Debug/ConsoleDrawer.h"
#include "Hud/Debug/NetworkStatusDrawer.h"
#include "Hud/HealthbarDrawer.h"

#include "Network/NetworkMessage.h"
#include "Network/ClientReplicator.h"
#include "Network/ClientManager.h"

#include "Camera/ICamera.h"
#include "EntitySystem/IEntityManager.h"
#include "EventHandler/EventHandler.h"
#include "Math/Vector.h"
#include "Rendering/Sprite/SpriteBatchDrawer.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Objects/StaticBackground.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "TransformSystem/TransformSystemDrawer.h"

#include "DamageSystem.h"
#include "GameCamera/CameraSystem.h"
#include "PredictionSystem/PositionPredictionSystem.h"
#include "PredictionSystem/PositionPredictionSystemDebug.h"
#include "PredictionSystem/SpawnPredictionSystem.h"
#include "Player/ClientPlayerDaemon.h"

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
    const std::function<mono::EventResult (const TransformMessage&)> transform_func = std::bind(&RemoteZone::HandleTransformMessage, this, _1);
    const std::function<mono::EventResult (const DamageInfoMessage&)> damage_func = std::bind(&RemoteZone::HandleDamageInfoMessage, this, _1);

    m_metadata_token = m_event_handler->AddListener(metadata_func);
    m_text_token = m_event_handler->AddListener(text_func);
    m_spawn_token = m_event_handler->AddListener(spawn_func);
    m_sprite_token = m_event_handler->AddListener(sprite_func);
    m_transform_token = m_event_handler->AddListener(transform_func);
    m_damageinfo_token = m_event_handler->AddListener(damage_func);
}

RemoteZone::~RemoteZone()
{
    m_event_handler->RemoveListener(m_metadata_token);
    m_event_handler->RemoveListener(m_text_token);
    m_event_handler->RemoveListener(m_spawn_token);
    m_event_handler->RemoveListener(m_sprite_token);
    m_event_handler->RemoveListener(m_transform_token);
    m_event_handler->RemoveListener(m_damageinfo_token);
}

void RemoteZone::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    m_camera = camera;

    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    m_sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();
    m_entity_manager = m_system_context->GetSystem<mono::IEntityManager>();

    m_damage_system = m_system_context->GetSystem<DamageSystem>();
    CameraSystem* camera_system = m_system_context->GetSystem<CameraSystem>();
    ClientManager* client_manager = m_system_context->GetSystem<ClientManager>();
    client_manager->StartClient();

    m_position_prediction_system =
        m_system_context->CreateSystem<PositionPredictionSystem>(500, client_manager, transform_system);

    m_spawn_prediction_system = m_system_context->CreateSystem<SpawnPredictionSystem>(
        client_manager, m_sprite_system, m_damage_system, m_position_prediction_system);

    m_player_daemon = std::make_unique<ClientPlayerDaemon>(camera_system, m_event_handler);
    m_debug_input = std::make_unique<ImGuiInputHandler>(*m_event_handler);
    m_console_drawer = std::make_unique<ConsoleDrawer>();

    AddUpdatable(new ClientReplicator(camera, client_manager));

    AddDrawable(new mono::SpriteBatchDrawer(transform_system, m_sprite_system), LayerId::GAMEOBJECTS);
    AddDrawable(new PredictionSystemDebugDrawer(m_position_prediction_system), LayerId::GAMEOBJECTS_DEBUG);
    AddDrawable(new mono::TransformSystemDrawer(g_draw_transformsystem, transform_system), LayerId::UI);
    AddDrawable(new HealthbarDrawer(m_damage_system, transform_system, m_entity_manager), LayerId::UI);
    AddDrawable(m_console_drawer.get(), LayerId::UI);
    AddDrawable(new DebugUpdater(nullptr, transform_system, m_event_handler), LayerId::UI);
    AddDrawable(new NetworkStatusDrawer(client_manager), LayerId::UI);
}

int RemoteZone::OnUnload()
{
    ClientManager* client_manager = m_system_context->GetSystem<ClientManager>();
    client_manager->Disconnect();

    RemoveDrawable(m_console_drawer.get());
    return SETUP_GAME_SCREEN;
}

mono::EventResult RemoteZone::HandleLevelMetadata(const LevelMetadataMessage& metadata_message)
{
    /*
    const auto component_filter = [](uint32_t component_hash) {
        return
            component_hash == TRANSFORM_COMPONENT ||
            component_hash == SPRITE_COMPONENT ||
            //component_hash == TEXT_COMPONENT ||
            component_hash == HEALTH_COMPONENT;
    };

    const char* world_filename = game::HashToFilename(metadata_message.world_file_hash);
    if(world_filename)
    {
        const shared::LevelData level_data =
            shared::ReadWorldComponentObjectsFiltered(world_filename, m_entity_manager, component_filter);

        m_camera->SetPosition(level_data.metadata.camera_position);
        m_camera->SetViewportSize(level_data.metadata.camera_size);

        if(!level_data.metadata.background_texture.empty())
            AddDrawable(new StaticBackground(level_data.metadata.background_texture.c_str()), LayerId::BACKGROUND);
    }
    else
    {
        System::Log("RemoteZone|Unable to load world file for hash %u", metadata_message.world_file_hash);
    }
    */

    m_camera->SetPosition(metadata_message.camera_position);
    m_camera->SetViewportSize(metadata_message.camera_size);

    const char* background_texture_filename = game::HashToFilename(metadata_message.background_texture_hash);
    if(background_texture_filename)
        AddDrawable(new mono::StaticBackground(background_texture_filename, mono::TextureModeFlags::REPEAT), LayerId::BACKGROUND);

    return mono::EventResult::HANDLED;
}

mono::EventResult RemoteZone::HandleText(const TextMessage& text_message)
{
    m_console_drawer->AddText(text_message.text, 1500);
    return mono::EventResult::HANDLED;
}

mono::EventResult RemoteZone::HandleSpawnMessage(const SpawnMessage& spawn_message)
{
    m_spawn_prediction_system->HandleSpawnMessage(spawn_message);
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

    const math::Vector shadow_offset(sprite_message.shadow_offset_x, sprite_message.shadow_offset_y);
    sprite->SetShadowOffset(shadow_offset);
    sprite->SetShadowSize(sprite_message.shadow_size);
    sprite->SetProperties(sprite_message.properties);

    m_sprite_system->SetSpriteLayer(sprite_message.entity_id, sprite_message.layer);

    return mono::EventResult::HANDLED;
}

mono::EventResult RemoteZone::HandleTransformMessage(const TransformMessage& transform_message)
{
    m_position_prediction_system->HandlePredicitonMessage(transform_message);
    return mono::EventResult::HANDLED;
}

mono::EventResult RemoteZone::HandleDamageInfoMessage(const DamageInfoMessage& damageinfo_message)
{
    const bool is_allocated = m_damage_system->IsAllocated(damageinfo_message.entity_id);
    if(!is_allocated)
        m_damage_system->CreateRecord(damageinfo_message.entity_id);

    DamageRecord* damage_record = m_damage_system->GetDamageRecord(damageinfo_message.entity_id);
    damage_record->health = damageinfo_message.health;
    damage_record->full_health = damageinfo_message.full_health;
    damage_record->is_boss = damageinfo_message.is_boss;
    damage_record->last_damaged_timestamp = damageinfo_message.damage_timestamp;

    return mono::EventResult::HANDLED;
}
