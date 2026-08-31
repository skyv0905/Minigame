#pragma once
#include <raylib.h>
#include <string>
#include <vector>
#include <unordered_map>

class ResourceManager;

class SoundPlayer
{
public:
    explicit SoundPlayer(ResourceManager& resourceManager);
    ~SoundPlayer();

    void Play(const std::string& key);
    void UnloadAll();

private:
    struct SoundPool
    {
        std::vector<Sound> voices;
        int nextVoice = 0;
    };

    ResourceManager& resourceManager;

    std::unordered_map<std::string, SoundPool> pools;

    static constexpr int MAX_VOICES = 8;
};