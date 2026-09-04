#pragma once

namespace Minigame::Server
{
    struct Vector2
    {
        float x = 0.0f;
        float y = 0.0f;
    };

    enum class ColliderType
    {
        None,
        Player,
        Wall,
        Mob,
        Bullet,
        PowerUp
    };

    struct ServerCollider
    {
        Vector2 size;
        Vector2 offset;
        bool isTrigger = false;
        ColliderType type = ColliderType::None;
    };

    enum class MobState
    {
        Regen,
        Idle,
        Hit
    };
}
