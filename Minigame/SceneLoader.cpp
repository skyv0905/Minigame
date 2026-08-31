#include "SceneLoader.h"
#include "SceneManager.h"
#include "Components/Transform.h"
#include "Components/SpriteRenderer.h"
#include "Components/PlayerController.h"
#include "Components/Button.h"
#include <fstream>
#include <algorithm>
#include <iostream>

SceneLoader::SceneLoader(GameServices& gameServices, GameObjectFactory& gameObjectFactory) : gameServices(gameServices), gameObjectFactory(gameObjectFactory)
{
}

std::vector<SceneInfo> SceneLoader::LoadSceneInfos()
{
    std::vector<SceneInfo> sceneInfos;
    std::ifstream file(std::string(GetApplicationDirectory()) + "Data/Scene/SceneList.json");
    std::cout << "Loading Scene Infos...\n";

    if (!file.is_open())
        return sceneInfos;

    json data;
    file >> data;

    for (const auto& sceneJson : data.at("scenes"))
    {
        SceneInfo info
        {
            sceneJson.at("name").get<std::string>(),
            sceneJson.at("index").get<int>()
        };
        sceneInfos.push_back(std::move(info));
    }

    sort(sceneInfos.begin(), sceneInfos.end(), [](const auto& a, const auto& b)
        {
            return a.index < b.index;
        });

    return sceneInfos;
}

std::unique_ptr<Scene> SceneLoader::LoadScene(const SceneInfo& info)
{
    std::ifstream file(std::string(GetApplicationDirectory()) + "Data/Scene/" + info.name + ".json");

    if (!file.is_open())
        return nullptr;

    json data;
    file >> data;

    auto scene = std::make_unique<Scene>(gameServices, gameObjectFactory, info.name, info.index);
    scene->SetBgm(data.at("bgm").get<std::string>());
    LoadGameObjects(*scene, data);

    return scene;
}

void SceneLoader::LoadGameObjects(Scene& scene, const json& data)
{
    if (!data.contains("gameObjects"))
        return;

    for (const auto& gameObjectData : data.at("gameObjects"))
    {
        auto gameObject = gameObjectFactory.Create(scene, gameObjectData);
        scene.AddGameObject(std::move(gameObject));
    }
}
