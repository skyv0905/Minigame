#include "MapBuilder.h"
#include "SpriteRenderer.h"
#include "Collider.h"
#include "TextUI.h"
#include "MobControllerNetwork.h"
#include "../GameServices.h"
#include "../ResourceManager.h"
#include "../GameSession.h"
#include "../SoundPlayer.h"
#include "../GameObject.h"
#include "../Scene.h"
#include "../RandomManager.h"
#include "Network/Packets.h"
#include <iostream>

namespace Minigame::Components
{
	MapBuilder::MapBuilder(GameObject& owner, GameServices& gameServices) :
		Component(owner),
		gameServices(gameServices)
	{
	}

	MapBuilder::~MapBuilder()
	{
		gameServices.timer.Cancel(mobSpawnTimer);
	}

	void MapBuilder::Start()
	{
	}

	void MapBuilder::Update(float deltaTime)
	{
		if (needBuild)
		{
			mapRandomGenerator = gameServices.random.CreateGenerator(RandomStream::Map);
			spawnRandomGenerator = gameServices.random.CreateGenerator(RandomStream::Spawn);

			Build();
			SpawnMobAndPowerUps();
			if (gameServices.session.HasNetworkMatch())
			{
				stageEnd = !stages.contains(gameServices.session.GetStage() + 1);
			}
			return;
		}

		if (gameServices.session.GetGameState() == GameState::GamePlaying && owner.GetScene().FindGameObjectWithTag("Mob") == nullptr)
		{
			if (gameServices.session.HasNetworkMatch())
				return;

			if (stageEnd)
			{
				OnGameCleared();
			}
			else
			{
				gameServices.timer.Cancel(mobSpawnTimer);
				OnStageEnded();
			}
		}
	}

	void MapBuilder::SetGrid(Vector2 grid)
	{
		this->grid = grid;
	}

	void MapBuilder::SetBlockSize(Vector2 size)
	{
		blockSize = size;
	}

	void MapBuilder::SetChunkSize(Vector2 size)
	{
		chunkSize = size;
	}

	void MapBuilder::AddStageInfo(int index, StageInfo&& info)
	{
		stages.emplace(index, std::move(info));
	}

	void MapBuilder::AddChunkPreset(std::vector<std::string>&& preset)
	{
		chunkPresets.push_back(std::move(preset));
	}

	void MapBuilder::AddPowerUpToList(std::string&& info)
	{
		powerUps.push_back(std::move(info));
	}

	void MapBuilder::Build()
	{
		theme = "yellowToyCastle.img";

		needBuild = false;
		nextNetworkObjectId = static_cast<std::uint32_t>(Minigame::Network::WorldStatePacket::MaxPlayers + 1);
		spawnPoints.clear();

		// outline
		const float width = grid.x * blockSize.x;
		const float height = grid.y * blockSize.y;
		playArea = { (1366 - width) * 0.5f, (768 - height) * 0.5f, width, height };
		const float offsetx = playArea.x - blockSize.x;
		const float offsety = playArea.y - blockSize.y;

		for (int y = 0; y < static_cast<int>(grid.y) + 2; y++)
		{
			for (int x = 0; x < static_cast<int>(grid.x) + 2; x++)
			{
				auto* outWall = owner.GetScene().Instantiate("OutWall");
				if (outWall == nullptr)
					return;

				Vector2 pos{ x * blockSize.x + offsetx, y * blockSize.y + offsety };
				auto* outWallTransform = outWall->GetComponent<Minigame::Components::Transform>();
				if (outWallTransform)
				{
					outWallTransform->SetPosition(pos);
				}

				auto* outWallSpriteRenderer = outWall->GetComponent<Minigame::Components::SpriteRenderer>();
				if (outWallSpriteRenderer)
				{
					if (const auto* texture = gameServices.resources.GetTexture(theme + "/bsc.0.png"))
					{
						outWallSpriteRenderer->SetTexture(*texture);
					}
				}

				auto* outWallCollider = outWall->GetComponent<Minigame::Components::Collider>();
				if (outWallCollider)
				{
					outWallCollider->SetSize(blockSize);
				}

				if (y > 0 && y < static_cast<int>(grid.y) + 1)
				{
					x += static_cast<int>(grid.x);
				}

			}
		}

		// inner: fill chunks
		if (chunkPresets.empty())
			return;

		std::size_t presetSize = chunkPresets.size();
		std::uniform_int_distribution<std::size_t> ud(0, presetSize - 1);
		std::bernoulli_distribution bd(0.5);

		for (int y = 0; y < static_cast<int>(grid.y); y += static_cast<int>(chunkSize.y))
		{
			for (int x = 0; x < static_cast<int>(grid.x); x += static_cast<int>(chunkSize.x))
			{
				const bool flipX = bd(mapRandomGenerator);
				const std::size_t index = ud(mapRandomGenerator);
				const auto& preset = chunkPresets[index];

				for (std::size_t cy = 0; cy < preset.size(); cy++)
				{
					const auto& row = preset[cy];
					for (std::size_t cx = 0; cx < row.size(); cx++)
					{
						const char& type = row[flipX ? row.size() - 1 - cx : cx];
						const int px = x + static_cast<int>(cx);
						const int py = y + static_cast<int>(cy);
						switch (type)
						{
						case 'W':
						{
							SpawnWall(px * blockSize.x + playArea.x, py * blockSize.y + playArea.y);
							break;
						}

						case 'E':
						{
							spawnPoints.push_back({ (px + 0.5f) * blockSize.x + playArea.x, (py + 0.5f) * blockSize.y + playArea.y });
							break;
						}
						}
					}
				}
			}
		}
	}

	void MapBuilder::SpawnWall(float x, float y)
	{
		auto* wall = owner.GetScene().Instantiate("Wall");
		if (wall == nullptr)
			return;

		Vector2 pos{ x, y };
		auto* wallTransform = wall->GetComponent<Minigame::Components::Transform>();
		if (wallTransform)
		{
			wallTransform->SetPosition(pos);
		}

		auto* wallSpriteRenderer = wall->GetComponent<Minigame::Components::SpriteRenderer>();
		if (wallSpriteRenderer)
		{
			if (const auto* texture = gameServices.resources.GetTexture(theme + "/bsc.0.png"))
			{
				wallSpriteRenderer->SetTexture(*texture);
			}
		}

		auto* wallCollider = wall->GetComponent<Minigame::Components::Collider>();
		if (wallCollider)
		{
			wallCollider->SetSize(blockSize);
		}
	}

	void MapBuilder::SpawnMobAndPowerUps()
	{
		if (spawnPoints.empty())
		{
			return;
		}

		const auto* currentStage = GetCurrentStage();
		if (!currentStage)
		{
			return;
		}

		std::shuffle(spawnPoints.begin(), spawnPoints.end(), spawnRandomGenerator);
		std::size_t i = 0;

		const std::size_t spawnCount = std::min<std::size_t>(currentStage->spawnCount, spawnPoints.size());
		const std::size_t powerUpSpawnCount = std::min<std::size_t>(currentStage->powerUpCount, spawnPoints.size() - spawnCount);

		if (!currentStage->mobs.empty())
		{
			std::size_t curMobIndex = 0;
			std::size_t maxIndex = currentStage->mobs.size() - 1;
			int remain = currentStage->mobs[curMobIndex].count;
			for (i; i < spawnCount;)
			{
				while (remain <= 0 && ++curMobIndex <= maxIndex)
				{
					remain = currentStage->mobs[curMobIndex].count;
				}
				if (remain-- <= 0) break;

				SpawnByPrefab(spawnPoints[i], currentStage->mobs[curMobIndex].prefab);
				i++;
			}
		}

		if (!powerUps.empty())
		{
			std::uniform_int_distribution<std::size_t> ud(0, powerUps.size() - 1);

			for (std::size_t powerUpIndex = 0; powerUpIndex < powerUpSpawnCount; powerUpIndex++)
			{
				SpawnByPrefab(spawnPoints[i + powerUpIndex], powerUps[ud(spawnRandomGenerator)]);
			}
		}

		gameServices.sounds.Play("Transform.mp3");

		if (!gameServices.session.HasNetworkMatch())
		{
			mobSpawnTimer = gameServices.timer.SetTimeout(currentStage->nextSpawnCooldown, [this]()
				{
					OnStageEnded();
				});
		}

	}

	void MapBuilder::ApplyStageChanged(const Minigame::Network::StageChangedPacket& packet)
	{
		if (!gameServices.session.HasNetworkMatch() || packet.stage != gameServices.session.GetStage() + 1)
			return;

		gameServices.session.SetStage(packet.stage);
		nextNetworkObjectId = packet.firstObjectId;
		SpawnMobAndPowerUps();
		const std::uint32_t objectCount = nextNetworkObjectId - packet.firstObjectId;
		if (objectCount != packet.objectCount)
		{
			std::cerr << "Stage object count mismatch: expected " << packet.objectCount << ", created " << objectCount << '\n';
		}
		stageEnd = !stages.contains(gameServices.session.GetStage() + 1);
	}

	void MapBuilder::SpawnByPrefab(const Vector2& pos, const std::string& prefab)
	{
		auto* object = owner.GetScene().Instantiate(prefab);
		if (object == nullptr)
			return;

		auto* transform = object->GetComponent<Minigame::Components::Transform>();
		if (transform)
		{
			transform->SetPosition(pos);
		}

		if (gameServices.session.HasNetworkMatch())
		{
			object->SetNetworkObjectId(nextNetworkObjectId);
			if (auto* networkController = object->GetComponent<Minigame::Components::MobControllerNetwork>())
			{
				networkController->SetObjectId(nextNetworkObjectId);
			}
			nextNetworkObjectId++;
		}
	}

	const StageInfo* MapBuilder::GetCurrentStage() const
	{
		auto it = stages.find(gameServices.session.GetStage());
		if (it == stages.end())
		{
			return nullptr;
		}

		return &it->second;
	}

	void MapBuilder::OnStageEnded()
	{
		if (!stages.contains(gameServices.session.GetStage() + 1)) // stage end
		{
			stageEnd = true;
		}
		else // goto next stage
		{
			gameServices.session.NextStage();
			SpawnMobAndPowerUps();
		}
	}

	void MapBuilder::OnGameCleared()
	{
		owner.GetScene().Instantiate("RestartButton");
		owner.GetScene().Instantiate("MainMenuButton");
		if (GameObject* textUIGameObject = owner.GetScene().Instantiate("TextUI"))
		{
			if (TextUI* textUI = textUIGameObject->GetComponent<TextUI>())
			{
				textUI->SetText("모든 스테이지 클리어!");
			}
		}

		gameServices.session.SetGameState(GameState::GameClear);
	}
}
