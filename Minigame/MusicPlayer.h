#pragma once
#include "ResourceManager.h"

class ResourceManager;

class MusicPlayer
{
public:
	explicit MusicPlayer(ResourceManager& resourceManager);

	void Play(const std::string& key);
	void Stop();
	void Update();

private:
	ResourceManager& resourceManager;
	Music* currentMusic = nullptr;
};

