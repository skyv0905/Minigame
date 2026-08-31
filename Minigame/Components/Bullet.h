#pragma once
#include <raylib.h>
#include "Component.h"
#include "PlayerController.h"
#include "Transform.h"
#include "Animator.h"
#include "Collider.h"

namespace Minigame::Components
{
	enum class CreatedFromInfo
	{
		None,
		Player,
		Mob
	};

	class Bullet : public Component
	{
	public:
		explicit Bullet(GameObject& owner);

		void Awake() override;
		void Start() override;
		void Update(float deltaTime) override;
		void OnCollisionEnter(const CollisionInfo& info) override;

		void SetMaxDistance(float dist);
		void SetMoveSpeed(float speed);
		void SetDirection(Vector2 dir);
		void SetCreatedFromInfo(CreatedFromInfo info);
		CreatedFromInfo GetCreatedFromInfo() const;
		void SetAttackPower(float power);
		float GetAttackPower() const;

	private:
		Minigame::Components::Transform* transform = nullptr;
		Minigame::Components::Animator* animator = nullptr;

		float movedDistance = 0.0f;
		float maxDistance = 300.0f;
		float moveSpeed = 500.0f;
		float attackPower = 0.0f;
		Vector2 direction{ 0.0f, 0.0f };
		CreatedFromInfo createdFromInfo = CreatedFromInfo::None;

		void InitRotation();
		bool rotationInited = false;

		bool isHit = false;
	};
}
