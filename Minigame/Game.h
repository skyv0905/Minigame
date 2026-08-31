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
	GameServices gameServices;

	SceneManager sceneManager;

	void Loop();
	void Update();
	void Draw();

	void InitInputManager();
};

