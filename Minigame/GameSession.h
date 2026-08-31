#pragma once

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

private:
    int stage = 1;
    GameState gameState = GameState::GamePlaying;
};