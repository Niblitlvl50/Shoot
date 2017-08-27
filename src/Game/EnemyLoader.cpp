
#include "EnemyLoader.h"
#include "Enemies/Enemy.h"
#include "System/File.h"
#include "Math/Vector.h"
#include "Math/Serialize.h"

#include "nlohmann_json/json.hpp"

std::vector<std::shared_ptr<game::Enemy>> game::LoadEnemies(const char* object_file, IEnemyFactory* factory)
{
    std::vector<std::shared_ptr<game::Enemy>> loaded_enemies;
    
    File::FilePtr file = File::OpenAsciiFile(object_file);
    if(!file)
        return loaded_enemies;

    std::vector<byte> file_data;
    File::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);
    const nlohmann::json& json_objects = json["objects"];

    for(const auto& json_object : json_objects)
    {
        const std::string& name = json_object["name"];
        const math::Vector& position = json_object["position"];
        const float rotation = json_object["rotation"];

        auto enemy = factory->CreateFromName(name.c_str(), position);
        if(enemy)
        {
            enemy->SetRotation(rotation);
            loaded_enemies.push_back(enemy);
        }
    }

    return loaded_enemies;
}
