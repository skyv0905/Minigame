#pragma once
#include <cstdint>
#include <string>
#include "Component.h"
#include "Transform.h"
#include "SpriteRenderer.h"
#include "Animator.h"
#include "Health.h"
#include "../TimerManager.h"

struct GameServices;
namespace Minigame::Network
{
	struct PlayerStatsChangedPacket;
}

namespace Minigame::Components
{
	class Controller : public Component
	{
	public:
		Controller(GameObject& owner, GameServices& gameServices);
		~Controller() override;

		void Awake() override;
		void Start() override;
		void Update(float deltaTime) override = 0;

		void SetBulletPrefab(const std::string& prefabName);
		void SetMoveSpeed(float speed);
		void SetBulletSpeed(float speed);
		void SetBulletDistance(float dist);
		void SetBulletTint(Color color);
		void SetFireCooldown(float cooldown);
		void SetAttackPower(float power);
		void SetNetworkStats(const Minigame::Network::PlayerStatsChangedPacket& packet);

		float GetFinalMoveSpeed() const;
		float GetFinalBulletSpeed() const;
		float GetFinalBulletDistance() const;
		float GetFinalAttackPower() const;
		float GetFireCooldown() const;
		Color GetBulletTint() const;
		std::uint32_t GetFireSequence() const;
		Vector2 GetForward() const;

		std::string GetMoveSpeedDetail() const;
		std::string GetBulletSpeedDetail() const;
		std::string GetBulletDistanceDetail() const;
		std::string GetAttackPowerDetail() const;

	protected:
		GameServices& gameServices;
		Minigame::Components::Transform* transform = nullptr;
		Minigame::Components::Animator* animator = nullptr;
		Minigame::Components::SpriteRenderer* spriteRenderer = nullptr;
		Minigame::Components::Health* health = nullptr;

		float moveSpeed = 100.0f;
		Vector2 forward{ 1.0f, 0.0f };

		std::string bulletPrefab;
		float bulletSpeed = 500.0f;
		float bulletDistance = 300.0f;
		Color bulletTint = WHITE;

		TimerId fireTimer = 0;
		bool canFire = true;
		float fireCooldown = 0.15f;

		float attackPower = 10.0f;
		std::uint32_t fireSequence = 0;

		int moveSpeedMultiplier = 100;
		int bulletSpeedMultiplier = 100;
		int bulletDistanceMultiplier = 100;
		int attackPowerMultiplier = 100;

		bool isOnRegen = false;
		bool isHit = false;
		bool isDead = false;

		virtual void Fire();
		void OnFireCooldownEnd();

		void DisableCollider();
		void EnableCollider();
	};
}
