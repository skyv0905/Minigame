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
    void ReloadCurrentScene();

private:
    GameServices& gameServices;
    GameObjectFactory gameObjectFactory;
    SceneLoader sceneLoader;

    std::unique_ptr<Scene> currentScene;
    std::vector<SceneInfo> sceneInfos;

    int currentSceneIndex = -1;
    int pendingSceneIndex = -1;

    void ApplyPendingSceneChange();
};

