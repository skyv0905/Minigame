#include "GameSession.h"

int GameSession::GetStage() const
{
    return stage;
}

void GameSession::NextStage()
{
    stage++;
}

void GameSession::Reset()
{
    stage = 1;
    gameState = GameState::GamePlaying;
}

void GameSession::SetGameState(GameState gameState)
{
    this->gameState = gameState;
}

GameState GameSession::GetGameState() const
{
    return gameState;
}
