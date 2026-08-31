#include "SoundPlayer.h"
#include "ResourceManager.h"

SoundPlayer::SoundPlayer(ResourceManager& resourceManager) : resourceManager(resourceManager)
{
}

void SoundPlayer::Play(const std::string& key)
{
    auto it = pools.find(key);

    if (it == pools.end())
    {
        Sound* source = resourceManager.GetSound(key);
        if (!source)
            return;

        SoundPool pool;
        pool.voices.reserve(MAX_VOICES);

        for (int i = 0; i < MAX_VOICES; i++)
        {
            pool.voices.push_back(LoadSoundAlias(*source));
        }

        auto [newIt, inserted] = pools.emplace(key, std::move(pool));

        it = newIt;
    }

    SoundPool& pool = it->second;
    for (Sound& voice : pool.voices)
    {
        if (!IsSoundPlaying(voice))
        {
            PlaySound(voice);
            return;
        }
    }

    Sound& voice = pool.voices[pool.nextVoice];
    PlaySound(voice);

    pool.nextVoice = (pool.nextVoice + 1) % MAX_VOICES;
}

void SoundPlayer::UnloadAll()
{
    for (auto& [key, pool] : pools)
    {
        for (Sound& voice : pool.voices)
        {
            UnloadSoundAlias(voice);
        }
    }

    pools.clear();
}

SoundPlayer::~SoundPlayer()
{
    UnloadAll();
}
