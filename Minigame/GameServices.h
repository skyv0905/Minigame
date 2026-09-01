#pragma once

class ResourceManager;
class MusicPlayer;
class SoundPlayer;
class InputManager;
class TimerManager;
class RandomManager;
class GameSession;
class IDGenerator;
namespace Minigame::Network
{
    class NetworkClient;
}

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
    Minigame::Network::NetworkClient& network;

    bool debugMode = false;
};
