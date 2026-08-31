#pragma once

class GameSession
{
public:
    int GetStage() const;
    void NextStage();
    void Reset();

private:
    int stage = 1;
};