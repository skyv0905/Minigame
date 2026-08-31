#pragma once

#include <cstdint>

using GameObjectID = std::uint64_t;

class IDGenerator
{
public:
	GameObjectID Generate();

private:
	GameObjectID nextID = 1;
};
