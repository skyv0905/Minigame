#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace Minigame::Server
{
    class ServerDataLoader
    {
    public:
        std::optional<nlohmann::json> LoadJson(const std::string& path) const;
    };
}
