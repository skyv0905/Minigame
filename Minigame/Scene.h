#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_set>
#include "GameServices.h"
#include "GameObject.h"
#include "Components/Collider.h"

struct GameServices;
class GameObjectFactory;

class Scene
{
public:
	Scene(GameServices& gameServices, GameObjectFactory& gameObjectFactory, std::string name, int index);

	void Start();
	void Update(float deltaTime);
	void Draw();

	void SetBgm(const std::string& bgm);
	const std::string& GetName() const;
	int GetIndex() const;
	void AddGameObject(std::unique_ptr<GameObject> gameObject);
	void DestroyGameObject(GameObject& gameObject);
	GameObject* Instantiate(const std::string& prefabName);

	GameObject* FindGameObject(const std::string& name);
	GameObject* FindGameObjectWithTag(const std::string& name);

private:
	GameServices& gameServices;
	GameObjectFactory& gameObjectFactory;
	std::string name;
	int index;

	std::string bgm;
	std::vector<std::unique_ptr<GameObject>> gameObjects;
	std::vector<std::unique_ptr<GameObject>> pendingGameObjects;
	std::unordered_set<GameObject*> pendingDestroyGameObjects;
	std::vector<Minigame::Components::Collider*> colliders;

	void CheckCollisions();
	void FlushPendingGameObjects();
};

