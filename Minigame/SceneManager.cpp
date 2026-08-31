#include "SceneManager.h"

SceneManager::SceneManager(GameServices& gameServices) : gameServices(gameServices), gameObjectFactory(gameServices, *this), sceneLoader(gameServices, gameObjectFactory)
{
}

void SceneManager::Update(float deltaTime)
{
	if (currentScene)
	{
		currentScene->Update(deltaTime);
	}

	ApplyPendingReload();
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
	scenes = sceneLoader.Load();
}

void SceneManager::SelectScene(int index)
{
	if (index >= 0 && index < scenes.size())
	{
		currentSceneSlot = index;
		currentScene = scenes[index].get();
		if (currentScene)
		{
			currentScene->Start();
		}
	}
}

void SceneManager::ReloadCurrentScene()
{
	if (currentScene && currentSceneSlot >= 0)
	{
		reloadRequested = true;
	}
}

void SceneManager::ApplyPendingReload()
{
	if (!reloadRequested || !currentScene || currentSceneSlot < 0)
		return;

	reloadRequested = false;

	const SceneInfo info{ currentScene->GetName(), currentScene->GetIndex() };
	auto reloadedScene = sceneLoader.LoadScene(info);
	if (!reloadedScene)
		return;

	scenes[currentSceneSlot] = std::move(reloadedScene);
	currentScene = scenes[currentSceneSlot].get();
	currentScene->Start();
}
