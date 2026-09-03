#include "ServerMapBuilder.h"
#include <algorithm>
#include <cmath>
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

    bool ServerMapBuilder::Build(const nlohmann::json& sceneData, const nlohmann::json& prefabData, ServerWorld& world, std::uint32_t randomSeed, int stage) const
    {
        if (!sceneData.contains("gameObjects") || !sceneData.at("gameObjects").is_array() || !prefabData.is_object())
            return false;

        ServerWorld builtWorld;
        std::vector<ServerPlayer> players;
        const nlohmann::json* mapBuilderData = nullptr;

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
            std::uint32_t nextObjectId = 1;
            for (const ServerPlayer& player : players)
            {
                if (player.playerId == 0 || !std::isfinite(player.position.x) || !std::isfinite(player.position.y) || !playerIds.insert(player.playerId).second)
                    return false;

                nextObjectId = std::max(nextObjectId, player.playerId + 1);
                builtWorld.AddPlayer(player);
            }

            if (!BuildMap(*mapBuilderData, prefabData, builtWorld, randomSeed, stage, nextObjectId))
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

    bool ServerMapBuilder::BuildMap(const nlohmann::json& componentData, const nlohmann::json& prefabData, ServerWorld& world, std::uint32_t randomSeed, int stage, std::uint32_t& nextObjectId) const
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
        std::vector<Vector2> spawnPoints;

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

        const nlohmann::json* stageData = nullptr;
        for (const auto& candidate : componentData.at("stages"))
        {
            if (candidate.value("stage", 0) == stage)
            {
                stageData = &candidate;
                break;
            }
        }
        if (stageData == nullptr || spawnPoints.empty())
            return false;

        std::mt19937 spawnGenerator = CreateGenerator(randomSeed, SpawnRandomStream);
        std::shuffle(spawnPoints.begin(), spawnPoints.end(), spawnGenerator);
        const std::size_t spawnCount = std::min<std::size_t>(stageData->value("spawnCount", 0), spawnPoints.size());
        std::size_t spawnIndex = 0;

        for (const auto& mobData : stageData->at("mobs"))
        {
            const std::string prefab = mobData.value("prefab", "");
            const int count = mobData.value("count", 0);
            for (int i = 0; i < count && spawnIndex < spawnCount; i++, spawnIndex++)
            {
                if (!AddMob(prefabData, prefab, spawnPoints[spawnIndex], nextObjectId++, world))
                    return false;
            }
        }

        const auto& powerUps = componentData.at("powerUps");
        if (!powerUps.is_array() || powerUps.empty())
            return stageData->value("powerUpCount", 0) == 0;

        const std::size_t powerUpCount = std::min<std::size_t>(stageData->value("powerUpCount", 0), spawnPoints.size() - spawnCount);
        std::uniform_int_distribution<std::size_t> powerUpDistribution(0, powerUps.size() - 1);
        for (std::size_t i = 0; i < powerUpCount; i++)
        {
            const std::string prefab = powerUps.at(powerUpDistribution(spawnGenerator)).get<std::string>();
            if (!AddPowerUp(prefabData, prefab, spawnPoints[spawnIndex + i], nextObjectId++, world))
                return false;
        }
        return true;
    }

    bool ServerMapBuilder::AddMob(const nlohmann::json& prefabData, const std::string& prefab, Vector2 position, std::uint32_t objectId, ServerWorld& world) const
    {
        if (!prefabData.contains(prefab))
            return false;

        const auto& data = prefabData.at(prefab);
        const auto* controller = FindComponent(data, "MobController");
        const auto* health = FindComponent(data, "Health");
        if (controller == nullptr || health == nullptr)
            return false;

        ServerMob mob{};
        mob.objectId = objectId;
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

    bool ServerMapBuilder::AddPowerUp(const nlohmann::json& prefabData, const std::string& prefab, Vector2 position, std::uint32_t objectId, ServerWorld& world) const
    {
        if (!prefabData.contains(prefab))
            return false;

        const auto& data = prefabData.at(prefab);
        ServerPowerUp powerUp{};
        powerUp.objectId = objectId;
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
                    powerUp.effect = tag;
                    break;
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
