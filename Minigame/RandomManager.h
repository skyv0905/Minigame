#pragma once

#include <cstdint>
#include <random>

enum class RandomStream : std::uint32_t
{
    Map = 1,
    Spawn,
};

class RandomManager
{
public:
    RandomManager();
    explicit RandomManager(std::uint32_t seed);

    void SetSeed(std::uint32_t seed);
    std::uint32_t GetSeed() const;

    std::mt19937 CreateGenerator(RandomStream stream) const;

private:
    std::uint32_t seed;
};
