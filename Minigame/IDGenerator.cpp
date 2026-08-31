#include "IDGenerator.h"

GameObjectID IDGenerator::Generate()
{
	return nextID++;
}
