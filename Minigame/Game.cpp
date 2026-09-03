#include "Game.h"
#include <raylib.h>
#include <iostream>

Game::Game() : musicPlayer(resourceManager),
    soundPlayer(resourceManager),
    gameServices{ resourceManager, musicPlayer, soundPlayer, inputManager, timerManager, randomManager, gameSession, idGenerator, networkClient },
    sceneManager(gameServices)
{
}

void Game::Run()
{
    InitInputManager();

    InitWindow(1366, 900, "ㅇㅅㅇ");
    SetExitKey(KEY_NULL);

    sceneManager.LoadScenes();

    Image icon = LoadImage(ResourceManager::GetResourcePath("icon.png").c_str());
    SetWindowIcon(icon);
    UnloadImage(icon);

    InitAudioDevice();

    SetTargetFPS(144);

    sceneManager.SelectScene(0);

    std::cout << "Init FInished, starting game loop...\n";

    while (!WindowShouldClose() && !exitRequested)
    {
        Loop();
    }

    std::cout << "Closing game...\n";

    musicPlayer.Stop();
    soundPlayer.UnloadAll();
    resourceManager.UnloadAll();
    CloseAudioDevice();
    CloseWindow();
}

void Game::Loop()
{
    Update();
    Draw();
}

void Game::Update()
{
    float deltaTime = GetFrameTime();

    if (inputManager.IsPressed(InputAction::Exit))
    {
        if (sceneManager.GetCurrentSceneIndex() == 0)
        {
            exitRequested = true;
            return;
        }

        sceneManager.SelectScene(0);
        gameSession.Reset();
    }
    if (inputManager.IsPressed(InputAction::Debug))
    {
        gameServices.debugMode = !gameServices.debugMode;
    }

    networkClient.Update(deltaTime);

    if (auto gameStart = networkClient.ConsumeGameStartPacket())
    {
        if (gameSession.GetPendingMultiScene() >= 0)
        {
            randomManager.SetSeed(gameStart->randomSeed);
            gameSession.Reset();
            gameSession.BeginNetworkMatch(gameStart->startTick);
        }
    }
    if (networkClient.ConsumeGameClosedPacket())
    {
        networkClient.Disconnect();
        gameSession.Reset();
        sceneManager.SelectScene(0);
    }

    const int pendingMultiScene = gameSession.GetPendingMultiScene();
    const bool waitingForGameStart = pendingMultiScene >= 0 && sceneManager.GetCurrentSceneIndex() == pendingMultiScene && !gameSession.HasNetworkMatch(); // 서버부터 게임 시작 대기 중

    musicPlayer.Update();
    sceneManager.Update(deltaTime, !waitingForGameStart);
    if (!waitingForGameStart)
    {
        timerManager.Update(deltaTime);
    }

	if (pendingMultiScene >= 0 && sceneManager.GetCurrentSceneIndex() == pendingMultiScene && !gameSession.IsMultiSceneReadySent()) // 씬 로딩 완료 후, 서버에 준비 완료 패킷 전송
    {
        Minigame::Network::PlayerReadyPacket readyPacket{};
        if (networkClient.SendPacket(readyPacket, Minigame::Network::PacketSendType::Reliable, Minigame::Network::PacketChannelType::Control))
        {
            gameSession.MarkMultiSceneReadySent();
        }
    }
}

void Game::Draw()
{
    BeginDrawing();

    ClearBackground(RAYWHITE);
    //DrawText("Hello raylib!", 100, 100, 30, BLACK);
    sceneManager.Draw();
    DrawText(TextFormat("Stage %d", gameSession.GetStage()), 10, 10, 15, BLACK);

    EndDrawing();
}

void Game::InitInputManager()
{
    inputManager.BindKey(InputAction::MoveLeft, KeyboardKey::KEY_LEFT);
    inputManager.BindKey(InputAction::MoveRight, KeyboardKey::KEY_RIGHT);
    inputManager.BindKey(InputAction::MoveUp, KeyboardKey::KEY_UP);
    inputManager.BindKey(InputAction::MoveDown, KeyboardKey::KEY_DOWN);
    inputManager.BindKey(InputAction::MoveLeft, KeyboardKey::KEY_A);
    inputManager.BindKey(InputAction::MoveRight, KeyboardKey::KEY_D);
    inputManager.BindKey(InputAction::MoveUp, KeyboardKey::KEY_W);
    inputManager.BindKey(InputAction::MoveDown, KeyboardKey::KEY_S);

    inputManager.BindKey(InputAction::Fire, KeyboardKey::KEY_SPACE);
    inputManager.BindKey(InputAction::Fire, KeyboardKey::KEY_LEFT_CONTROL);

    inputManager.BindKey(InputAction::Debug, KeyboardKey::KEY_F1);
    inputManager.BindKey(InputAction::Exit, KeyboardKey::KEY_ESCAPE);
}
