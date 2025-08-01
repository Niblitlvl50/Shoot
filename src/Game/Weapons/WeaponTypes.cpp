
#include "WeaponTypes.h"
#include "Weapons/Serialize.h"

#include "System/Hash.h"
#include "System/File.h"

#include "nlohmann/json.hpp"

game::WeaponConfig game::LoadWeaponConfig(const char* weapon_config_file)
{
    WeaponConfig weapon_config;

    file::FilePtr file = file::OpenAsciiFile(weapon_config_file);
    if(!file)
        return weapon_config;

    const std::vector<byte> file_data = file::FileRead(file);
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    weapon_config.weapon_pickup_entity = json.value("weapon_pickup_entity", "");

    for(const BulletConfiguration bullet : json["bullets"])
        weapon_config.bullet_configs[hash::Hash(bullet.name.c_str())] =  bullet;

    for(const WeaponConfiguration weapon : json["weapons"])
        weapon_config.weapon_configs[hash::Hash(weapon.name.c_str())] =  weapon;

    for(const WeaponBulletCombination weapon_bullet_combo : json["weapon_setups"])
    {
        weapon_config.weapon_names.push_back(weapon_bullet_combo.name);
        weapon_config.weapon_combinations[hash::Hash(weapon_bullet_combo.name.c_str())] = weapon_bullet_combo;
    }

    weapon_config.weapon_levels = json.value("weapon_levels", std::vector<int>());

    return weapon_config;
}

game::WeaponSetup game::FindWeaponSetupFromString(const WeaponConfig& weapon_config, const char* weapon_combination_name)
{
    const uint32_t weapon_name_hash = hash::Hash(weapon_combination_name);
    WeaponSetup weapon_setup = { weapon_name_hash, 0, 0 };

    const auto it = weapon_config.weapon_combinations.find(weapon_name_hash);
    if(it != weapon_config.weapon_combinations.end())
    {
        weapon_setup.weapon_hash = hash::Hash(it->second.weapon.c_str());
        weapon_setup.bullet_hash = hash::Hash(it->second.bullet.c_str());
    }
    
    return weapon_setup;
}

game::ModifiersConfig game::LoadModifiersConfig(const char* modifiers_config_file)
{
    ModifiersConfig config;

    file::FilePtr file = file::OpenAsciiFile(modifiers_config_file);
    if(!file)
        return config;

    const std::vector<byte> file_data = file::FileRead(file);
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    for(const auto& modifier_json : json["modifiers"])
    {
        const std::string& name = modifier_json.value("name", "");
        const std::string& sprite = modifier_json.value("sprite_file", "");

        config.modifier_id_to_sprite.insert_or_assign(hash::Hash(name.c_str()), sprite);
    }

    return config;
}
