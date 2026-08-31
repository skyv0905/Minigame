#pragma once
#include <string>
#include "Component.h"
#include "Transform.h"
#include "SpriteRenderer.h"
#include "Animator.h"
#include "Health.h"
#include "../TimerManager.h"

struct GameServices;

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

		float GetFinalMoveSpeed() const;
		float GetFinalBulletSpeed() const;
		float GetFinalBulletDistance() const;
		float GetFinalAttackPower() const;

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

		float moveSpeedMultiplier = 1.0f;
		float bulletSpeedMultiplier = 1.0f;
		float bulletDistanceMultiplier = 1.0f;
		float attackPowerMultiplier = 1.0f;

		bool isOnRegen = false;
		bool isHit = false;
		bool isDead = false;

		virtual void Fire();
		void OnFireCooldownEnd();

		void DisableCollider();
		void EnableCollider();
	};
}
