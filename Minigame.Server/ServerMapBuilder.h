#pragma once

#include "ServerWorld.h"
#include <cstdint>
#include <nlohmann/json.hpp>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace Minigame::Server
{
    struct ServerMobSpawnInfo
    {
        std::string prefab;
        int count = 0;
    };

    struct ServerStageInfo
    {
        int spawnCount = 0;
        float nextSpawnCooldown = 0.0f;
        int powerUpCount = 0;
        std::vector<ServerMobSpawnInfo> mobs;
    };

    struct StageSpawnResult
    {
        std::uint32_t firstObjectId = 0;
        std::uint16_t objectCount = 0;
    };

    class ServerMapBuilder
    {
    public:
        bool Build(const nlohmann::json& sceneData, const nlohmann::json& prefabData, ServerWorld& world, std::uint32_t randomSeed, int stage = 1);
        bool SpawnMobAndPowerUps(ServerWorld& world, int stage, StageSpawnResult& result);
        bool HasStage(int stage) const;
        float GetNextSpawnCooldown(int stage) const;

    private:
        nlohmann::json prefabData;
        std::unordered_map<int, ServerStageInfo> stages;
        std::vector<Vector2> spawnPoints;
        std::vector<std::string> powerUps;
        std::mt19937 spawnRandomGenerator;

        bool ReadPlayerSpawner(const nlohmann::json& componentData, const nlohmann::json& prefabData, std::vector<ServerPlayer>& players) const;
        bool BuildMap(const nlohmann::json& componentData, ServerWorld& world, std::uint32_t randomSeed);
        bool AddMob(const std::string& prefab, Vector2 position, ServerWorld& world) const;
        bool AddPowerUp(const std::string& prefab, Vector2 position, ServerWorld& world) const;
        bool ReadCollider(const nlohmann::json& prefab, ServerCollider& collider) const;
    };
}
