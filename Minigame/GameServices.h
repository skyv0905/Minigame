#pragma once

class ResourceManager;
class MusicPlayer;
class SoundPlayer;
class InputManager;
class TimerManager;
class RandomManager;
class GameSession;
class IDGenerator;

struct GameServices
{
    ResourceManager& resources;
    MusicPlayer& music;
    SoundPlayer& sounds;
    InputManager& input;
    TimerManager& timer;
    RandomManager& random;
    GameSession& session;
    IDGenerator& idGenerator;

    bool debugMode = false;
};
