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

std::vector<std::unique_ptr<Scene>> SceneLoader::Load()
{
    std::vector<std::unique_ptr<Scene>> loadedScenes;
    std::ifstream file(std::string(GetApplicationDirectory()) + "Data/Scene/SceneList.json");
    std::cout << "Loading Scenes...\n";

    if (!file.is_open())
        return loadedScenes;

    json data;
    file >> data;

    std::vector<SceneInfo> toLoadScenes;
    for (const auto& sceneJson : data.at("scenes"))
    {
        SceneInfo info
        {
            sceneJson.at("name").get<std::string>(),
            sceneJson.at("index").get<int>()
        };
        toLoadScenes.push_back(info);
    }

    sort(toLoadScenes.begin(), toLoadScenes.end(), [](const auto& a, const auto& b)
        {
            return a.index < b.index;
        });

    for (const auto& info : toLoadScenes)
    {
        std::cout << "  Scene " + std::to_string(info.index) + ": " + info.name + "...\n";
        auto scene = LoadScene(info);
        if (scene)
        {
            loadedScenes.push_back(std::move(scene));
            std::cout << "  ...succeeded\n";
        }
        else
        {
            std::cout << "  ...failed\n";
        }
    }

    return loadedScenes;
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
