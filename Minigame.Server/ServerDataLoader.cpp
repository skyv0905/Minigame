#include "ServerDataLoader.h"
#include <fstream>

namespace Minigame::Server
{
    std::optional<nlohmann::json> ServerDataLoader::LoadJson(const std::string& path) const
    {
        std::ifstream file(path);
        if (!file.is_open())
            return std::nullopt;

        try
        {
            nlohmann::json data;
            file >> data;
            return data;
        }
        catch (const nlohmann::json::exception&)
        {
            return std::nullopt;
        }
    }
}
