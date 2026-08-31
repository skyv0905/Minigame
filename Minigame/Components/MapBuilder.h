#pragma once
#include "Component.h"
#include "../TimerManager.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <raylib.h>

struct GameServices;

namespace Minigame::Components
{
	struct MobInfo
	{
		std::string prefab;
		int count = 0;
	};

	struct StageInfo
	{
		int spawnCount = 0;
		float nextSpawnCooldown = 0.0f;
		int powerUpCount = 0;
		std::vector<MobInfo> mobs;
	};

	class MapBuilder : public Component
	{
	public:
		MapBuilder(GameObject& owner, GameServices& gameServices);
		~MapBuilder() override;

		void Start() override;
		void Update(float deltaTime) override;

		void SetGrid(Vector2 grid);
		void SetBlockSize(Vector2 size);
		void SetChunkSize(Vector2 size);

		void AddStageInfo(int index, StageInfo&& info);
		void AddChunkPreset(std::vector<std::string>&& preset);
		void AddPowerUpToList(std::string&& info);
		
		void Build();

	private:
		GameServices& gameServices;

		Vector2 grid{ 15, 12 };
		Vector2 blockSize{ 90, 60 };
		Vector2 chunkSize{ 5, 3 };
		std::unordered_map<int, StageInfo> stages;

		std::vector<std::vector<std::string>> chunkPresets;

		std::mt19937 mapRandomGenerator;
		std::mt19937 spawnRandomGenerator;

		std::string theme;
		bool needBuild = true;

		Rectangle playArea{};

		std::vector<Vector2> spawnPoints;
		TimerId mobSpawnTimer = 0;

		std::vector<std::string> powerUps;

		bool stageEnd = false;

		void SpawnWall(float x, float y);

		void SpawnMobAndPowerUps();
		void SpawnByPrefab(const Vector2& pos, const std::string& mobPrefab);

		const StageInfo* GetCurrentStage() const;

		void OnStageEnded();
		void OnGameCleared();
	};
}

