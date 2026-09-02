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
