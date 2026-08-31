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
	ReloadScene(currentSceneSlot);
}

void SceneManager::ReloadScene(int index)
{
	if (index >= 0 && index < scenes.size())
	{
		reloadSceneSlot = index;
	}
}

void SceneManager::ApplyPendingReload()
{
	if (reloadSceneSlot < 0 || reloadSceneSlot >= scenes.size())
		return;

	const int sceneSlot = reloadSceneSlot;
	reloadSceneSlot = -1;

	const SceneInfo info{ scenes[sceneSlot]->GetName(), scenes[sceneSlot]->GetIndex() };
	auto reloadedScene = sceneLoader.LoadScene(info);
	if (!reloadedScene)
		return;

	scenes[sceneSlot] = std::move(reloadedScene);
	currentSceneSlot = sceneSlot;
	currentScene = scenes[sceneSlot].get();
	currentScene->Start();
}
