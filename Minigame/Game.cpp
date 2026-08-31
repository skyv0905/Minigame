#include "Game.h"
#include <raylib.h>
#include <iostream>

Game::Game() : musicPlayer(resourceManager),
    soundPlayer(resourceManager),
    gameServices{ resourceManager, musicPlayer, soundPlayer, inputManager, timerManager, randomManager, gameSession },
    sceneManager(gameServices)
{
}

void Game::Run()
{
    InitInputManager();

    InitWindow(1366, 900, "ㅇㅅㅇ");

    sceneManager.LoadScenes();

    Image icon = LoadImage(ResourceManager::GetResourcePath("icon.png").c_str());
    SetWindowIcon(icon);
    UnloadImage(icon);

    InitAudioDevice();

    SetTargetFPS(144);

    sceneManager.SelectScene(0);

    std::cout << "Init FInished, starting game loop...\n";

    while (!WindowShouldClose())
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

    musicPlayer.Update();
    sceneManager.Update(deltaTime);
    timerManager.Update(deltaTime);

    if (inputManager.IsPressed(InputAction::Debug))
    {
        gameServices.debugMode = !gameServices.debugMode;
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

    inputManager.BindKey(InputAction::Fire, KeyboardKey::KEY_SPACE);
    inputManager.BindKey(InputAction::Fire, KeyboardKey::KEY_LEFT_CONTROL);

    inputManager.BindKey(InputAction::Debug, KeyboardKey::KEY_F1);
}
