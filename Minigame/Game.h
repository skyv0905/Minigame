#pragma once
#include "ResourceManager.h"
#include "MusicPlayer.h"
#include "SoundPlayer.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "TimerManager.h"
#include "GameSession.h"
#include "GameServices.h"
#include "RandomManager.h"
#include "IDGenerator.h"
#include "Network/NetworkClient.h"

class Game
{
public:
	Game();

	void Run();

private:
	ResourceManager resourceManager;
	MusicPlayer musicPlayer;
	SoundPlayer soundPlayer;
	InputManager inputManager;
	TimerManager timerManager;
	RandomManager randomManager;
	GameSession gameSession;
	IDGenerator idGenerator;
	Minigame::Network::NetworkClient networkClient;
	GameServices gameServices;

	SceneManager sceneManager;

	bool exitRequested = false;

	void Loop();
	void Update();
	void Draw();

	void InitInputManager();
};

