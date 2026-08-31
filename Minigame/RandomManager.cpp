#include "RandomManager.h"

RandomManager::RandomManager() : RandomManager(std::random_device{}())
{
}

RandomManager::RandomManager(std::uint32_t seed) : seed(seed)
{
}

void RandomManager::SetSeed(std::uint32_t seed)
{
    this->seed = seed;
}

std::uint32_t RandomManager::GetSeed() const
{
    return seed;
}

std::mt19937 RandomManager::CreateGenerator(RandomStream stream) const
{
    std::seed_seq seedSequence
    {
        seed,
        static_cast<std::uint32_t>(stream)
    };

    return std::mt19937(seedSequence);
}
