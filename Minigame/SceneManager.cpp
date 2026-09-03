#include "SceneManager.h"
#include <algorithm>

SceneManager::SceneManager(GameServices& gameServices) : gameServices(gameServices), gameObjectFactory(gameServices, *this), sceneLoader(gameServices, gameObjectFactory)
{
}

void SceneManager::Update(float deltaTime, bool updateCurrentScene)
{
	if (currentScene && updateCurrentScene)
	{
		currentScene->Update(deltaTime);
	}

	ApplyPendingSceneChange();
}

void SceneManager::Draw()
{
	if (currentScene)
	{
		currentScene->Draw();
	}
}

void SceneManager::LoadScenes()
{
	sceneInfos = sceneLoader.LoadSceneInfos();
}

void SceneManager::SelectScene(int index)
{
	const auto sceneInfo = std::find_if(sceneInfos.begin(), sceneInfos.end(), [index](const SceneInfo& info)
		{
			return info.index == index;
		});

	if (sceneInfo != sceneInfos.end())
	{
		pendingSceneIndex = index;
	}
}

void SceneManager::ReloadCurrentScene()
{
	SelectScene(currentSceneIndex);
}

int SceneManager::GetCurrentSceneIndex() const
{
	return currentSceneIndex;
}

void SceneManager::ApplyBulletSpawn(const Minigame::Network::BulletSpawnPacket& packet)
{
	if (currentScene)
	{
		currentScene->ApplyBulletSpawn(packet);
	}
}

void SceneManager::ApplyBulletDestroy(const Minigame::Network::BulletDestroyPacket& packet)
{
	if (currentScene)
	{
		currentScene->ApplyBulletDestroy(packet);
	}
}

void SceneManager::ApplyExpChanged(const Minigame::Network::ExpChangedPacket& packet)
{
	if (currentScene)
	{
		currentScene->ApplyExpChanged(packet);
	}
}

void SceneManager::ApplyHpChanged(const Minigame::Network::HpChangedPacket& packet)
{
	if (currentScene)
	{
		currentScene->ApplyHpChanged(packet);
	}
}

void SceneManager::ApplyGameResult(const Minigame::Network::GameResultPacket& packet, std::uint32_t localPlayerId)
{
	if (currentScene)
	{
		currentScene->ApplyGameResult(packet, localPlayerId);
	}
}

void SceneManager::ApplyPendingSceneChange()
{
	if (pendingSceneIndex < 0)
		return;

	const int sceneIndex = pendingSceneIndex;
	pendingSceneIndex = -1;

	const auto sceneInfo = std::find_if(sceneInfos.begin(), sceneInfos.end(), [sceneIndex](const SceneInfo& info)
		{
			return info.index == sceneIndex;
		});
	if (sceneInfo == sceneInfos.end())
		return;

	auto loadedScene = sceneLoader.LoadScene(*sceneInfo);
	if (!loadedScene)
		return;

	currentScene = std::move(loadedScene);
	currentSceneIndex = sceneIndex;
	currentScene->Start();
}
