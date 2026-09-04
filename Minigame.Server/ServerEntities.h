#pragma once

#include "ServerTypes.h"
#include <cstdint>
#include <deque>
#include <string>
#include <unordered_set>

namespace Minigame::Server
{
    struct ServerPlayerInput
    {
        float moveX = 0.0f;
        float moveY = 0.0f;
    };

    struct ServerPlayerFireRequest
    {
        std::uint32_t fireSequence = 0;
        std::uint32_t clientTick = 0;
        Vector2 position;
        Vector2 direction;
    };

    struct ServerPositionSnapshot
    {
        std::uint32_t serverTick = 0;
        Vector2 position;
    };

    struct ServerHealth
    {
        float maxHealth = 0.0f;
        float currentHealth = 0.0f;
    };

    struct ServerExp
    {
        int currentExp = 0;
        int requiredExp = 100;
        int level = 1;
        float requiredExpGrowthRate = 1.05f;
    };

    struct ServerPlayer
    {
        std::uint32_t playerId = 0;
        Vector2 position;
        Vector2 forward{ 1.0f, 0.0f };
        ServerCollider collider;
        ServerCollider bulletCollider;
        ServerPlayerInput input;
        std::deque<ServerPlayerFireRequest> fireRequests;
        std::deque<ServerPositionSnapshot> positionHistory;
        ServerHealth health;
        ServerExp exp;
        float moveSpeed = 150.0f;
        float bulletSpeed = 500.0f;
        float bulletDistance = 300.0f;
        float fireCooldown = 0.15f;
        float fireCooldownRemaining = 0.0f;
        float attackPower = 10.0f;
        int moveSpeedMultiplier = 100;
        int bulletSpeedMultiplier = 100;
        int bulletDistanceMultiplier = 100;
        int attackPowerMultiplier = 100;
        std::uint32_t lastProcessedFireSequence = 0;
    };

    struct ServerWall
    {
        Vector2 position;
        ServerCollider collider;
    };

    struct ServerMob
    {
        static constexpr float RegenDuration = 0.48f;
        static constexpr float HitDuration = 0.3f;

        std::uint32_t objectId = 0;
        std::uint32_t targetPlayerId = 0;
        std::string prefab;
        Vector2 position;
        Vector2 forward{ 1.0f, 0.0f };
        ServerCollider collider;
        ServerCollider bulletCollider;
        float speed = 0.0f;
        float bulletSpeed = 0.0f;
        float bulletDistance = 0.0f;
        float fireCooldown = 0.0f;
        float fireCooldownRemaining = 0.0f;
        float attackPower = 0.0f;
        float detectionRange = 0.0f;
        ServerHealth health;
        int exp = 0;
        MobState state = MobState::Regen;
        float stateRemaining = RegenDuration;
    };

    struct ServerBullet
    {
        std::uint32_t objectId = 0;
        std::uint32_t createdFrom = 0;
        std::uint32_t fireSequence = 0;
        ColliderType createdFromType = ColliderType::None;
        Vector2 position;
        Vector2 spawnPosition;
        Vector2 direction;
        ServerCollider collider;
        float movedDistance = 0.0f;
        float pendingMoveDistance = -1.0f;
        float maxDistance = 0.0f;
        float moveSpeed = 0.0f;
        float attackPower = 0.0f;
    };

    struct ServerPowerUp
    {
        std::uint32_t objectId = 0;
        std::string prefab;
        std::unordered_set<std::string> effects;
        Vector2 position;
        ServerCollider collider;
    };
}
