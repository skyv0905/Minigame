#pragma once
#include <vector>
#include "Scene.h"
#include "GameServices.h"
#include "SceneLoader.h"
#include "GameObjectFactory.h"

class SceneManager
{
public:
    explicit SceneManager(GameServices& gameServices);

    void Update(float deltaTime);
    void Draw();

    void LoadScenes();
    void SelectScene(int index);
    void ReloadScene(int index);
    void ReloadCurrentScene();

private:
    GameServices& gameServices;
    GameObjectFactory gameObjectFactory;
    SceneLoader sceneLoader;

    Scene* currentScene = nullptr;
    std::vector<std::unique_ptr<Scene>> scenes;

    int currentSceneSlot = -1;
    int reloadSceneSlot = -1;

    void ApplyPendingReload();
};

