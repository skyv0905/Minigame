#pragma once

class ResourceManager;
class MusicPlayer;
class SoundPlayer;
class InputManager;
class TimerManager;
class RandomManager;
class GameSession;

struct GameServices
{
    ResourceManager& resources;
    MusicPlayer& music;
    SoundPlayer& sounds;
    InputManager& input;
    TimerManager& timer;
    RandomManager& random;
    GameSession& session;

    bool debugMode = false;
};
