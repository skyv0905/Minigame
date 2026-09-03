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

    void Update(float deltaTime, bool updateCurrentScene = true);
    void Draw();

    void LoadScenes();
    void SelectScene(int index);
    void ReloadCurrentScene();

    int GetCurrentSceneIndex() const;
    void ApplyBulletSpawn(const Minigame::Network::BulletSpawnPacket& packet);
    void ApplyBulletDestroy(const Minigame::Network::BulletDestroyPacket& packet);

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

