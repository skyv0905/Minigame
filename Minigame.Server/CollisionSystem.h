#pragma once

#include <vector>

namespace Minigame::Server
{
    class ServerWorld;
    struct ServerCollider;
    struct Vector2;

    class CollisionSystem
    {
    public:
        void Update(ServerWorld& world);

    private:
        struct CollisionObject
        {
            Vector2* position = nullptr;
            ServerCollider* collider = nullptr;
        };

        void HandleCollision(CollisionObject& objectA, CollisionObject& objectB, Vector2 direction, float depth);
        std::vector<CollisionObject> CollectCollisionObjects(ServerWorld& world);
    };
}
