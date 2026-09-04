#include "ServerMapBuilder.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <nlohmann/json.hpp>
#include <random>
#include <unordered_set>
#include <utility>

namespace Minigame::Server
{
    namespace
    {
        constexpr float ScreenWidth = 1366.0f;
        constexpr float ScreenHeight = 768.0f;
        constexpr std::uint32_t MapRandomStream = 1;
        constexpr std::uint32_t SpawnRandomStream = 2;

        std::mt19937 CreateGenerator(std::uint32_t seed, std::uint32_t stream)
        {
            std::seed_seq seedSequence{ seed, stream };
            return std::mt19937(seedSequence);
        }

        Vector2 ReadVector2(const nlohmann::json& data)
        {
            return Vector2{ data.at("x").get<float>(), data.at("y").get<float>() };
        }

        const nlohmann::json* FindComponent(const nlohmann::json& gameObjectData, const std::string& type)
        {
            if (!gameObjectData.contains("components") || !gameObjectData.at("components").is_array())
                return nullptr;

            for (const auto& componentData : gameObjectData.at("components"))
            {
                if (componentData.value("type", "") == type)
                    return &componentData;
            }
            return nullptr;
        }
    }

    bool ServerMapBuilder::Build(const nlohmann::json& sceneData, const nlohmann::json& prefabData, ServerWorld& world, std::uint32_t randomSeed, int stage)
    {
        if (!sceneData.contains("gameObjects") || !sceneData.at("gameObjects").is_array() || !prefabData.is_object())
            return false;

        ServerWorld builtWorld;
        std::vector<ServerPlayer> players;
        const nlohmann::json* mapBuilderData = nullptr;
        this->prefabData = prefabData;
        stages.clear();
        spawnPoints.clear();
        powerUps.clear();
        spawnRandomGenerator = CreateGenerator(randomSeed, SpawnRandomStream);

        try
        {
            for (const auto& gameObjectData : sceneData.at("gameObjects"))
            {
                if (!gameObjectData.contains("components") || !gameObjectData.at("components").is_array())
                    continue;

                for (const auto& componentData : gameObjectData.at("components"))
                {
                    const std::string type = componentData.value("type", "");
                    if (type == "PlayerSpawner" && !ReadPlayerSpawner(componentData, prefabData, players))
                        return false;
                    if (type == "MapBuilder")
                        mapBuilderData = &componentData;
                }
            }

            if (players.empty() || mapBuilderData == nullptr)
                return false;

            std::unordered_set<std::uint32_t> playerIds;
            for (const ServerPlayer& player : players)
            {
                if (player.playerId == 0 || !std::isfinite(player.position.x) || !std::isfinite(player.position.y) || !playerIds.insert(player.playerId).second)
                    return false;

                builtWorld.AddPlayer(player);
            }

            if (!BuildMap(*mapBuilderData, builtWorld, randomSeed))
                return false;

            StageSpawnResult result{};
            if (!SpawnMobAndPowerUps(builtWorld, stage, result))
                return false;
        }
        catch (const nlohmann::json::exception&)
        {
            return false;
        }

        world = std::move(builtWorld);
        return true;
    }

    bool ServerMapBuilder::ReadPlayerSpawner(const nlohmann::json& componentData, const nlohmann::json& prefabData, std::vector<ServerPlayer>& players) const
    {
        if (!componentData.contains("players") || !componentData.at("players").is_array() || !prefabData.contains("Player"))
            return false;

        ServerCollider playerCollider{};
        if (!ReadCollider(prefabData.at("Player"), playerCollider))
            return false;

        const auto* controller = FindComponent(prefabData.at("Player"), "PlayerController");
        const auto* health = FindComponent(prefabData.at("Player"), "Health");
        const auto* experience = FindComponent(prefabData.at("Player"), "Exp");
        if (controller == nullptr || health == nullptr || experience == nullptr)
            return false;

        const std::string bulletPrefab = controller->value("bulletPrefab", "");
        ServerCollider bulletCollider{};
        if (!prefabData.contains(bulletPrefab) || !ReadCollider(prefabData.at(bulletPrefab), bulletCollider))
            return false;

        const auto& playerDataList = componentData.at("players");
        const std::size_t playerCount = componentData.value("playerCount", playerDataList.size());
        if (playerCount == 0 || playerCount > playerDataList.size())
            return false;

        for (std::size_t i = 0; i < playerCount; i++)
        {
            const auto& playerData = playerDataList.at(i);
            ServerPlayer player{};
            player.playerId = playerData.at("playerId").get<std::uint32_t>();
            player.position = ReadVector2(playerData.at("position"));
            player.collider = playerCollider;
            player.bulletCollider = bulletCollider;
            player.health.maxHealth = std::max(0.0f, health->value("maxHealth", 0.0f));
            player.health.currentHealth = player.health.maxHealth;
            player.exp.level = std::max(1, experience->value("initialLevel", 1));
            player.exp.requiredExp = std::max(1, experience->value("requiredExp", 100));
            player.exp.requiredExpGrowthRate = std::max(1.0f, experience->value("requiredExpGrowthRate", 1.05f));
            player.moveSpeed = controller->value("speed", 150.0f);
            player.bulletSpeed = controller->value("bulletSpeed", 500.0f);
            player.bulletDistance = controller->value("bulletDistance", 300.0f);
            player.fireCooldown = controller->value("fireCooldown", 0.15f);
            player.attackPower = controller->value("attackPower", 10.0f);
            players.push_back(player);
        }
        return true;
    }

    bool ServerMapBuilder::BuildMap(const nlohmann::json& componentData, ServerWorld& world, std::uint32_t randomSeed)
    {
        // load datas
        const Vector2 grid = ReadVector2(componentData.at("grid"));
        const Vector2 blockSize = ReadVector2(componentData.at("blockSize"));
        const Vector2 chunkSize = ReadVector2(componentData.at("chunkSize"));
        if (grid.x <= 0.0f || grid.y <= 0.0f || blockSize.x <= 0.0f || blockSize.y <= 0.0f || chunkSize.x <= 0.0f || chunkSize.y <= 0.0f)
            return false;

        const auto& presetData = componentData.at("chunkPreset");
        if (!presetData.is_array() || presetData.empty())
            return false;

        std::vector<std::vector<std::string>> chunkPresets;
        for (const auto& preset : presetData)
        {
            std::vector<std::string> rows;
            for (int rowIndex = 0;; rowIndex++)
            {
                const std::string key = std::to_string(rowIndex);
                if (!preset.contains(key))
                    break;
                rows.push_back(preset.at(key).get<std::string>());
            }
            if (rows.empty())
                return false;
            chunkPresets.push_back(std::move(rows));
        }

        // outline
        const float width = grid.x * blockSize.x;
        const float height = grid.y * blockSize.y;
        const Vector2 playArea{ (ScreenWidth - width) * 0.5f, (ScreenHeight - height) * 0.5f };
        const float offsetX = playArea.x - blockSize.x;
        const float offsetY = playArea.y - blockSize.y;

        for (int y = 0; y < static_cast<int>(grid.y) + 2; y++)
        {
            for (int x = 0; x < static_cast<int>(grid.x) + 2; x++)
            {
                world.AddWall(ServerWall
                    {
                        Vector2{ x * blockSize.x + offsetX, y * blockSize.y + offsetY },
                        ServerCollider{ blockSize }
                    });

                if (y > 0 && y < static_cast<int>(grid.y) + 1)
                {
                    x += static_cast<int>(grid.x);
                }
            }
        }

        // inner: fill chunks
        std::mt19937 mapGenerator = CreateGenerator(randomSeed, MapRandomStream);
        std::size_t presetSize = chunkPresets.size();
        std::uniform_int_distribution<std::size_t> ud(0, presetSize - 1);
        std::bernoulli_distribution bd(0.5);
        spawnPoints.clear();

        for (int y = 0; y < static_cast<int>(grid.y); y += static_cast<int>(chunkSize.y))
        {
            for (int x = 0; x < static_cast<int>(grid.x); x += static_cast<int>(chunkSize.x))
            {
                const bool flipX = bd(mapGenerator);
                const std::size_t index = ud(mapGenerator);
                const auto& preset = chunkPresets[index];

                for (std::size_t cy = 0; cy < preset.size(); cy++)
                {
                    const auto& row = preset[cy];
                    for (std::size_t cx = 0; cx < row.size(); cx++)
                    {
                        const char type = row[flipX ? row.size() - 1 - cx : cx];
                        const int px = x + static_cast<int>(cx);
                        const int py = y + static_cast<int>(cy);
                        switch (type)
                        {
                        case 'W':
                        {
                            world.AddWall(ServerWall
                                {
                                    Vector2{ px * blockSize.x + playArea.x, py * blockSize.y + playArea.y },
                                    ServerCollider{ blockSize }
                                });
                            break;
                        }

                        case 'E':
                        {
                            spawnPoints.push_back(Vector2{ (px + 0.5f) * blockSize.x + playArea.x, (py + 0.5f) * blockSize.y + playArea.y });
                            break;
                        }
                        }
                    }
                }
            }
        }

        if (!componentData.contains("stages") || !componentData.at("stages").is_array())
            return false;

        for (const auto& candidate : componentData.at("stages"))
        {
            ServerStageInfo stageInfo{};
            const int stage = candidate.value("stage", 0);
            stageInfo.spawnCount = candidate.value("spawnCount", 0);
            stageInfo.nextSpawnCooldown = candidate.value("nextSpawnCooldown", 0.0f);
            stageInfo.powerUpCount = candidate.value("powerUpCount", 0);
            if (stage <= 0 || !candidate.contains("mobs") || !candidate.at("mobs").is_array())
                return false;
            for (const auto& mobData : candidate.at("mobs"))
            {
                stageInfo.mobs.push_back(ServerMobSpawnInfo{ mobData.value("prefab", ""), mobData.value("count", 0) });
            }
            stages.insert_or_assign(stage, std::move(stageInfo));
        }
        if (stages.empty() || spawnPoints.empty())
            return false;

        if (!componentData.contains("powerUps") || !componentData.at("powerUps").is_array())
            return false;
        for (const auto& powerUp : componentData.at("powerUps"))
        {
            powerUps.push_back(powerUp.get<std::string>());
        }
        return true;
    }

    bool ServerMapBuilder::SpawnMobAndPowerUps(ServerWorld& world, int stage, StageSpawnResult& result)
    {
        const auto stageEntry = stages.find(stage);
        if (stageEntry == stages.end() || spawnPoints.empty())
            return false;

        const ServerStageInfo& stageInfo = stageEntry->second;
        std::shuffle(spawnPoints.begin(), spawnPoints.end(), spawnRandomGenerator);
        const std::size_t spawnCount = std::min<std::size_t>(stageInfo.spawnCount, spawnPoints.size());
        std::size_t spawnIndex = 0;
        result.firstObjectId = world.GetNextObjectId();

        for (const ServerMobSpawnInfo& mobInfo : stageInfo.mobs)
        {
            for (int i = 0; i < mobInfo.count && spawnIndex < spawnCount; i++, spawnIndex++)
            {
                if (!AddMob(mobInfo.prefab, spawnPoints[spawnIndex], world))
                    return false;
            }
        }

        if (!powerUps.empty())
        {
            const std::size_t powerUpCount = std::min<std::size_t>(stageInfo.powerUpCount, spawnPoints.size() - spawnCount);
            std::uniform_int_distribution<std::size_t> distribution(0, powerUps.size() - 1);
            for (std::size_t i = 0; i < powerUpCount; i++)
            {
                if (!AddPowerUp(powerUps[distribution(spawnRandomGenerator)], spawnPoints[spawnIndex + i], world))
                    return false;
            }
        }

        const std::uint32_t objectCount = world.GetNextObjectId() - result.firstObjectId;
        if (objectCount > (std::numeric_limits<std::uint16_t>::max)())
            return false;

        result.objectCount = static_cast<std::uint16_t>(objectCount);
        return true;
    }

    bool ServerMapBuilder::HasStage(int stage) const
    {
        return stages.contains(stage);
    }

    float ServerMapBuilder::GetNextSpawnCooldown(int stage) const
    {
        const auto entry = stages.find(stage);
        return entry == stages.end() ? 0.0f : entry->second.nextSpawnCooldown;
    }

    bool ServerMapBuilder::AddMob(const std::string& prefab, Vector2 position, ServerWorld& world) const
    {
        if (!prefabData.contains(prefab))
            return false;

        const auto& data = prefabData.at(prefab);
        const auto* controller = FindComponent(data, "MobController");
        const auto* health = FindComponent(data, "Health");
        if (controller == nullptr || health == nullptr)
            return false;

        ServerMob mob{};
        mob.objectId = world.AllocateObjectId();
        mob.prefab = prefab;
        mob.position = position;
        mob.speed = controller->value("speed", 100.0f);
        mob.bulletSpeed = controller->value("bulletSpeed", 500.0f);
        mob.bulletDistance = controller->value("bulletDistance", 300.0f);
        mob.fireCooldown = controller->value("fireCooldown", 5.0f);
        mob.attackPower = controller->value("attackPower", 10.0f);
        mob.detectionRange = controller->value("detectionRange", 100.0f);
        mob.exp = controller->value("exp", 0);
        mob.health.maxHealth = std::max(0.0f, health->at("maxHealth").get<float>());
        mob.health.currentHealth = mob.health.maxHealth;
        const std::string bulletPrefab = controller->value("bulletPrefab", "");
        if (!ReadCollider(data, mob.collider) || !prefabData.contains(bulletPrefab) || !ReadCollider(prefabData.at(bulletPrefab), mob.bulletCollider))
            return false;

        world.AddMob(std::move(mob));
        return true;
    }

    bool ServerMapBuilder::AddPowerUp(const std::string& prefab, Vector2 position, ServerWorld& world) const
    {
        if (!prefabData.contains(prefab))
            return false;

        const auto& data = prefabData.at(prefab);
        ServerPowerUp powerUp{};
        powerUp.objectId = world.AllocateObjectId();
        powerUp.prefab = prefab;
        powerUp.position = position;
        powerUp.collider.isTrigger = true;
        if (!ReadCollider(data, powerUp.collider))
            return false;

        if (data.contains("tags") && data.at("tags").is_array())
        {
            for (const auto& tagData : data.at("tags"))
            {
                const std::string tag = tagData.get<std::string>();
                if (tag != "PowerUp")
                {
                    powerUp.effects.insert(tag);
                }
            }
        }

        world.AddPowerUp(std::move(powerUp));
        return true;
    }

    bool ServerMapBuilder::ReadCollider(const nlohmann::json& prefab, ServerCollider& collider) const
    {
        const auto* colliderData = FindComponent(prefab, "Collider");
        if (colliderData == nullptr || !colliderData->contains("size"))
            return false;

        collider.size = ReadVector2(colliderData->at("size"));
        if (colliderData->contains("offset"))
        {
            collider.offset = ReadVector2(colliderData->at("offset"));
        }

        return collider.size.x > 0.0f && collider.size.y > 0.0f;
    }
}
