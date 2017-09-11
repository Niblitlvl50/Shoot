
#include "JsonSerializer.h"

#include "ObjectProxies/EntityProxy.h"
#include "ObjectProxies/PathProxy.h"
#include "ObjectProxies/PolygonProxy.h"
#include "ObjectProxies/PrefabProxy.h"

#include "Objects/Path.h"

#include "System/File.h"
#include "Paths/IPath.h"
#include "Paths/PathFactory.h"

#include "nlohmann_json/json.hpp"

using namespace editor;

JsonSerializer::JsonSerializer(const std::string& file_path)
    : m_file_path(file_path)
{ }

void JsonSerializer::WritePathFile()
{
    nlohmann::json json;
    json["path_files"] = m_path_names;

    const std::string& serialized_json = json.dump(4);

    File::FilePtr file = File::CreateAsciiFile(m_file_path.c_str());
    std::fwrite(serialized_json.data(), serialized_json.length(), sizeof(char), file.get());
}

void JsonSerializer::Accept(EntityProxy* proxy)
{

}

void JsonSerializer::Accept(PathProxy* proxy)
{
    auto path = proxy->m_path;

    const std::string& filename = "res/paths/" + path->m_name + ".path";
    mono::SavePath(filename.c_str(), path->Position(), path->m_points);

    m_path_names.push_back(filename);
}

void JsonSerializer::Accept(PolygonProxy* proxy)
{

}

void JsonSerializer::Accept(PrefabProxy* proxy)
{

}

// ----------

JsonDeserializer::JsonDeserializer(const std::string& file_path)
{

}

void JsonDeserializer::Accept(EntityProxy* proxy)
{

}

void JsonDeserializer::Accept(PathProxy* proxy)
{

}

void JsonDeserializer::Accept(PolygonProxy* proxy)
{

}

void JsonDeserializer::Accept(PrefabProxy* proxy)
{

}
