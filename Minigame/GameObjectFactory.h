#pragma once
#include <memory>
#include <string>
#include <nlohmann/json.hpp>
#include "Components/Animator.h"

struct GameServices;
class Scene;
class GameObject;
class SceneManager;

using json = nlohmann::json;

class GameObjectFactory
{
public:
    GameObjectFactory(GameServices& gameServices, SceneManager& sceneManager);

    std::unique_ptr<GameObject> CreatePrefab(Scene& scene, const std::string& name);
    std::unique_ptr<GameObject> Create(Scene& scene, const json& data);

private:
    GameServices& gameServices;
    SceneManager& sceneManager;
    json prefabData;
    json animationData;

    void LoadComponents(GameObject& gameObject, const json& data);
    void LoadComponent(GameObject& gameObject, const json& data);
    Minigame::Components::AnimationClip CreateAnimationClip(const std::string& state);
};

