#include "MusicPlayer.h"

MusicPlayer::MusicPlayer(ResourceManager& resourceManager) : resourceManager(resourceManager)
{
}

void MusicPlayer::Play(const std::string& key)
{
    Stop();

    currentMusic = resourceManager.GetMusic(key);
    if (!currentMusic)
        return;

    PlayMusicStream(*currentMusic);
}

void MusicPlayer::Stop()
{
    if (currentMusic == nullptr)
        return;

    StopMusicStream(*currentMusic);
    currentMusic = nullptr;
}

void MusicPlayer::Update()
{
    if (currentMusic != nullptr)
    {
        UpdateMusicStream(*currentMusic);
    }
}
