#include "GameSession.h"

int GameSession::GetStage() const
{
    return stage;
}

void GameSession::SetStage(int stage)
{
    this->stage = stage;
}

void GameSession::NextStage()
{
    stage++;
}

void GameSession::Reset()
{
    stage = 1;
    gameState = GameState::GamePlaying;
    pendingMultiScene = -1;
    multiSceneReadySent = false;
    matchStartServerTick = 0;
    networkMatchActive = false;
}

void GameSession::SetGameState(GameState gameState)
{
    this->gameState = gameState;
}

GameState GameSession::GetGameState() const
{
    return gameState;
}

void GameSession::SetPendingMultiScene(int sceneIndex)
{
	pendingMultiScene = sceneIndex;
	multiSceneReadySent = false;
}

int GameSession::GetPendingMultiScene() const
{
    return pendingMultiScene;
}

int GameSession::ConsumePendingMultiScene()
{
	int sceneIndex = pendingMultiScene;
	pendingMultiScene = -1;
	return sceneIndex;
}

bool GameSession::IsMultiSceneReadySent() const
{
    return multiSceneReadySent;
}

void GameSession::MarkMultiSceneReadySent()
{
    multiSceneReadySent = true;
}

void GameSession::BeginNetworkMatch(std::uint32_t startServerTick)
{
    matchStartServerTick = startServerTick;
    networkMatchActive = true;
}

bool GameSession::HasNetworkMatch() const
{
    return networkMatchActive;
}

std::uint32_t GameSession::GetMatchStartServerTick() const
{
    return matchStartServerTick;
}

std::uint32_t GameSession::GetElapsedMatchTicks(std::uint32_t currentServerTick) const
{
    if (!networkMatchActive)
        return 0;

    return currentServerTick - matchStartServerTick;
}
