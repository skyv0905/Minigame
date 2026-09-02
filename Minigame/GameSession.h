#pragma once
#include <cstdint>

enum class GameState
{
    GamePlaying,
    GameOver,
    GameClear
};

class GameSession
{
public:
    int GetStage() const;
    void NextStage();
    void Reset();

    void SetGameState(GameState gameState);
    GameState GetGameState() const;

    void SetPendingMultiScene(int sceneIndex);
    int ConsumePendingMultiScene();

    void BeginNetworkMatch(std::uint32_t startServerTick);
    bool HasNetworkMatch() const;
    std::uint32_t GetMatchStartServerTick() const;
    std::uint32_t GetElapsedMatchTicks(std::uint32_t currentServerTick) const;

private:
    int stage = 1;
    GameState gameState = GameState::GamePlaying;
    int pendingMultiScene = -1;
    std::uint32_t matchStartServerTick = 0;
    bool networkMatchActive = false;
};
