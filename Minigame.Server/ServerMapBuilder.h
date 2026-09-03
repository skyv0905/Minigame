#pragma once

#include "ServerWorld.h"
#include <cstdint>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

namespace Minigame::Server
{
    class ServerMapBuilder
    {
    public:
        bool Build(const nlohmann::json& sceneData, const nlohmann::json& prefabData, ServerWorld& world, std::uint32_t randomSeed, int stage = 1) const;

    private:
        bool ReadPlayerSpawner(const nlohmann::json& componentData, const nlohmann::json& prefabData, std::vector<ServerPlayer>& players) const;
        bool BuildMap(const nlohmann::json& componentData, const nlohmann::json& prefabData, ServerWorld& world, std::uint32_t randomSeed, int stage, std::uint32_t& nextObjectId) const;
        bool AddMob(const nlohmann::json& prefabData, const std::string& prefab, Vector2 position, std::uint32_t objectId, ServerWorld& world) const;
        bool AddPowerUp(const nlohmann::json& prefabData, const std::string& prefab, Vector2 position, std::uint32_t objectId, ServerWorld& world) const;
        bool ReadCollider(const nlohmann::json& prefab, ServerCollider& collider) const;
    };
}
