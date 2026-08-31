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
}
