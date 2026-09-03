#pragma once

namespace Minigame::Server
{
    struct ServerPlayer;

    class ExpSystem
    {
    public:
        void AddExp(ServerPlayer& player, int amount) const;

    private:
        void OnLevelUp(ServerPlayer& player) const;
    };
}
