#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "GameServices.h"
#include "GameObject.h"
#include "IDGenerator.h"
#include "Components/Collider.h"
#include "Network/Packets.h"

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
	GameObject* FindGameObjectByID(GameObjectID id);
	void ApplyBulletSpawn(const Minigame::Network::BulletSpawnPacket& packet);
	void ApplyBulletDestroy(const Minigame::Network::BulletDestroyPacket& packet);
	void ApplyExpChanged(const Minigame::Network::ExpChangedPacket& packet);
	void ApplyHpChanged(const Minigame::Network::HpChangedPacket& packet);
	void ApplyPlayerStatsChanged(const Minigame::Network::PlayerStatsChangedPacket& packet);
	void ApplyPowerUpCollected(const Minigame::Network::PowerUpCollectedPacket& packet);
	void ApplyStageChanged(const Minigame::Network::StageChangedPacket& packet);
	void ApplyGameResult(const Minigame::Network::GameResultPacket& packet, std::uint32_t localPlayerId);

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
	std::unordered_map<GameObjectID, GameObject*> gameObjectsByID;

	void CheckCollisions();
	void FlushPendingGameObjects();
	GameObject* FindNetworkObject(std::uint32_t objectId);
};

