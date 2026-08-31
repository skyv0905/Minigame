#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "GameServices.h"
#include "Scene.h"
#include "SceneInfo.h"

class GameObject;
class GameObjectFactory;

using json = nlohmann::json;

class SceneLoader
{
public:
    SceneLoader(GameServices& gameServices, GameObjectFactory& gameObjectFactory);

    std::vector<std::unique_ptr<Scene>> Load();
    std::unique_ptr<Scene> LoadScene(const SceneInfo& info);

private:
    GameServices& gameServices;
    GameObjectFactory& gameObjectFactory;

    void LoadGameObjects(Scene& scene, const json& data);
};
